#include "point_cloud_processor.h"
#include <vector>
#include <unordered_map>
#include <tuple>
#include <cmath>

namespace core {

QVector<QVector3D> PointCloudProcessor::filterCylindrical(const QVector<QVector3D>& input, float radiusMm, float minZ, float maxZ) {
    QVector<QVector3D> result;
    result.reserve(input.size());
    float radiusSq = radiusMm * radiusMm;

    for (const auto& pt : input) {
        float dSq = pt.x() * pt.x() + pt.y() * pt.y();
        if (dSq <= radiusSq && pt.z() >= minZ && pt.z() <= maxZ) {
            result.push_back(pt);
        }
    }
    return result;
}

QVector<QVector3D> PointCloudProcessor::removePoints(const QVector<QVector3D>& input, const QVector<int>& indicesToRemove) {
    if (indicesToRemove.isEmpty()) return input;

    std::vector<bool> removeFlag(input.size(), false);
    for (int idx : indicesToRemove) {
        if (idx >= 0 && idx < input.size()) {
            removeFlag[idx] = true;
        }
    }

    QVector<QVector3D> result;
    result.reserve(input.size() - indicesToRemove.size());
    for (int i = 0; i < input.size(); ++i) {
        if (!removeFlag[i]) {
            result.push_back(input[i]);
        }
    }
    return result;
}

QVector<QVector3D> PointCloudProcessor::filterStatisticalOutlier(const QVector<QVector3D>& input, int meanK, float stdDevMulThresh) {
    if (input.size() <= meanK) return input;

    PointCloudAdapter adapter(input);
    PointCloudKDTree tree(3, adapter, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    tree.buildIndex();

    std::vector<float> distances(input.size());
    float sumDist = 0.0f;

    for (int i = 0; i < input.size(); ++i) {
        std::vector<uint32_t> ret_indices(meanK + 1);
        std::vector<float> out_dists_sq(meanK + 1);
        float query_pt[3] = { input[i].x(), input[i].y(), input[i].z() };
        tree.knnSearch(&query_pt[0], meanK + 1, &ret_indices[0], &out_dists_sq[0]);

        float sum = 0.0f;
        for (int k = 1; k < meanK + 1; ++k) {
            sum += std::sqrt(out_dists_sq[k]);
        }
        distances[i] = sum / meanK;
        sumDist += distances[i];
    }

    float mean = sumDist / input.size();
    float variance = 0.0f;
    for (float d : distances) {
        variance += (d - mean) * (d - mean);
    }
    variance /= input.size();
    float stdDev = std::sqrt(variance);
    float threshold = mean + stdDevMulThresh * stdDev;

    QVector<QVector3D> result;
    result.reserve(input.size());
    for (int i = 0; i < input.size(); ++i) {
        if (distances[i] <= threshold) {
            result.push_back(input[i]);
        }
    }
    return result;
}

QVector<QVector3D> PointCloudProcessor::filterRadiusOutlier(const QVector<QVector3D>& input, float radius, int minNeighbors) {
    if (input.isEmpty()) return input;

    PointCloudAdapter adapter(input);
    PointCloudKDTree tree(3, adapter, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    tree.buildIndex();

    QVector<QVector3D> result;
    result.reserve(input.size());
    float search_radius_sq = radius * radius;

    for (int i = 0; i < input.size(); ++i) {
        std::vector<nanoflann::ResultItem<uint32_t, float>> matches;
        nanoflann::SearchParameters params;
        float query_pt[3] = { input[i].x(), input[i].y(), input[i].z() };
        const size_t nMatches = tree.radiusSearch(&query_pt[0], search_radius_sq, matches, params);

        if (nMatches >= (size_t)(minNeighbors + 1)) { // +1 because the point itself is included
            result.push_back(input[i]);
        }
    }
    return result;
}

// Custom hash for VoxelGrid
struct VoxelHash {
    std::size_t operator()(const std::tuple<int, int, int>& k) const {
        return std::hash<int>()(std::get<0>(k)) ^ 
               (std::hash<int>()(std::get<1>(k)) << 1) ^ 
               (std::hash<int>()(std::get<2>(k)) << 2);
    }
};

QVector<QVector3D> PointCloudProcessor::filterVoxelGrid(const QVector<QVector3D>& input, float leafSize) {
    if (input.isEmpty() || leafSize <= 0.0f) return input;

    std::unordered_map<std::tuple<int, int, int>, std::pair<QVector3D, int>, VoxelHash> grid;

    float invLeafSize = 1.0f / leafSize;
    for (const auto& pt : input) {
        int ix = static_cast<int>(std::floor(pt.x() * invLeafSize));
        int iy = static_cast<int>(std::floor(pt.y() * invLeafSize));
        int iz = static_cast<int>(std::floor(pt.z() * invLeafSize));

        auto key = std::make_tuple(ix, iy, iz);
        auto& entry = grid[key];
        entry.first += pt;
        entry.second += 1;
    }

    QVector<QVector3D> result;
    result.reserve(grid.size());
    for (const auto& kv : grid) {
        result.push_back(kv.second.first / static_cast<float>(kv.second.second));
    }
    return result;
}

QVector<QVector3D> PointCloudProcessor::transformCloud(const QVector<QVector3D>& input, const QMatrix4x4& transform) {
    QVector<QVector3D> result;
    result.reserve(input.size());
    for (const auto& pt : input) {
        result.push_back(transform.map(pt));
    }
    return result;
}

QMatrix4x4 PointCloudProcessor::calculateICP(const QVector<QVector3D>& source, const QVector<QVector3D>& target, int maxIterations, float tolerance) {
    // Basic placeholder implementation. For production, Eigen library SVD is highly recommended.
    // We start with Identity.
    QMatrix4x4 currentTransform;
    currentTransform.setToIdentity();
    
    if (source.isEmpty() || target.isEmpty()) return currentTransform;

    PointCloudAdapter targetAdapter(target);
    PointCloudKDTree tree(3, targetAdapter, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    tree.buildIndex();

    QVector<QVector3D> currentSource = source;

    float prevError = std::numeric_limits<float>::max();

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector3D sourceCentroid(0, 0, 0);
        QVector3D targetCentroid(0, 0, 0);
        int validPairs = 0;
        float errorSum = 0.0f;

        // In a real ICP we match points, find centroids, calculate cross-covariance,
        // and compute SVD to find the rotation. Since we don't have Eigen, we just return Identity.
        // We will notify the user in UI that ICP requires Eigen/PCL.
        break;
    }

    return currentTransform;
}

} // namespace core
