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


QMatrix4x4 PointCloudProcessor::calculateICP(const QVector<QVector3D>& source, const QVector<QVector3D>& target, bool isInverse, int maxIterations, float tolerance) {
    QMatrix4x4 currentTransform;
    currentTransform.setToIdentity();
    
    if (source.isEmpty() || target.isEmpty()) return currentTransform;

    // 1. Seyreltme (Subsampling) - 2mm voxel ile
    QVector<QVector3D> subSource = filterVoxelGrid(source, 2.0f);
    QVector<QVector3D> subTarget = filterVoxelGrid(target, 2.0f);

    if (subSource.size() < 10 || subTarget.size() < 10) {
        // Cok az nokta varsa orijinali kullanalim (kk veri)
        subSource = source;
        subTarget = target;
    }

    // 2. Initial Transform (Ters Tarama / CCW -> CW)
    // isInverse true ise Y ekseni ayna goruntusu yapilir
    if (isInverse) {
        for (auto& p : subSource) {
            p.setY(-p.y());
        }
        QMatrix4x4 flipMat;
        flipMat.scale(1.0f, -1.0f, 1.0f);
        currentTransform = flipMat * currentTransform;
    }

    PointCloudAdapter targetAdapter(subTarget);
    PointCloudKDTree tree(3, targetAdapter, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    tree.buildIndex();

    QVector<QVector3D> currentSource = subSource;
    float prevError = std::numeric_limits<float>::max();

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector<QVector3D> matchedSource;
        QVector<QVector3D> matchedTarget;

        float errorSum = 0.0f;
        
        // Eslestirme
        for (const auto& p : currentSource) {
            size_t ret_index;
            float out_dist_sqr;
            nanoflann::KNNResultSet<float> resultSet(1);
            resultSet.init(&ret_index, &out_dist_sqr);
            
            float query_pt[3] = {p.x(), p.y(), p.z()};
            tree.findNeighbors(resultSet, &query_pt[0], nanoflann::SearchParameters(10));
            
            if (out_dist_sqr < 25.0f) { // Maks 5mm uzakliktakiler (25 = 5^2)
                matchedSource.push_back(p);
                matchedTarget.push_back(subTarget[ret_index]);
                errorSum += out_dist_sqr;
            }
        }

        int N = matchedSource.size();
        if (N < 10) break; // Eslenen nokta cok az, ICP calismaz

        float meanError = errorSum / N;
        if (std::abs(prevError - meanError) < tolerance) {
            break; // Tolerans altina indik, dur
        }
        prevError = meanError;

        // Centroids
        QVector3D centroidSrc(0,0,0), centroidTgt(0,0,0);
        for (int i=0; i<N; ++i) {
            centroidSrc += matchedSource[i];
            centroidTgt += matchedTarget[i];
        }
        centroidSrc /= N;
        centroidTgt /= N;

        // Cross-covariance matrix
        float Sxx = 0, Sxy = 0, Sxz = 0;
        float Syx = 0, Syy = 0, Syz = 0;
        float Szx = 0, Szy = 0, Szz = 0;

        for (int i=0; i<N; ++i) {
            QVector3D p = matchedSource[i] - centroidSrc;
            QVector3D q = matchedTarget[i] - centroidTgt;
            Sxx += p.x()*q.x(); Sxy += p.x()*q.y(); Sxz += p.x()*q.z();
            Syx += p.y()*q.x(); Syy += p.y()*q.y(); Syz += p.y()*q.z();
            Szx += p.z()*q.x(); Szy += p.z()*q.y(); Szz += p.z()*q.z();
        }

        // 4x4 Symmetric Matrix N for Horn's Method
        float N11 = Sxx + Syy + Szz;
        float N12 = Syz - Szy;
        float N13 = Szx - Sxz;
        float N14 = Sxy - Syx;

        float N22 = Sxx - Syy - Szz;
        float N23 = Sxy + Syx;
        float N24 = Szx + Sxz;

        float N33 = -Sxx + Syy - Szz;
        float N34 = Syz + Szy;

        float N44 = -Sxx - Syy + Szz;

        float N_mat[4][4] = {
            {N11, N12, N13, N14},
            {N12, N22, N23, N24},
            {N13, N23, N33, N34},
            {N14, N24, N34, N44}
        };

        // Power Iteration to find principal eigenvector (quaternion)
        float q[4] = {1, 0, 0, 0};
        for (int pi = 0; pi < 30; ++pi) {
            float q_new[4] = {0,0,0,0};
            for (int i=0; i<4; ++i) {
                for (int j=0; j<4; ++j) {
                    q_new[i] += N_mat[i][j] * q[j];
                }
            }
            float norm = std::sqrt(q_new[0]*q_new[0] + q_new[1]*q_new[1] + q_new[2]*q_new[2] + q_new[3]*q_new[3]);
            if (norm < 1e-8f) break;
            q[0] = q_new[0]/norm; q[1] = q_new[1]/norm; q[2] = q_new[2]/norm; q[3] = q_new[3]/norm;
        }

        // Convert quaternion to Rotation Matrix
        QQuaternion rot(q[0], q[1], q[2], q[3]);
        QMatrix4x4 rotMat;
        rotMat.rotate(rot);

        // Translate
        QVector3D trans = centroidTgt - rotMat.map(centroidSrc);

        QMatrix4x4 stepTransform;
        stepTransform.translate(trans);
        stepTransform *= rotMat;

        // Apply to currentSource and accumulate transform
        for (auto& p : currentSource) {
            p = stepTransform.map(p);
        }
        currentTransform = stepTransform * currentTransform;
    }

    return currentTransform;
}


QVector<QVector3D> PointCloudProcessor::generateCylindricalMesh(const QVector<QVector3D>& input, int angleSteps, int zSteps) {
    if (input.isEmpty()) return QVector<QVector3D>();

    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& p : input) {
        if (p.z() < minZ) minZ = p.z();
        if (p.z() > maxZ) maxZ = p.z();
    }
    if (maxZ == minZ) maxZ += 1.0f; 

    std::vector<std::vector<float>> grid(zSteps, std::vector<float>(angleSteps, -1.0f));

    for (const auto& p : input) {
        float r = std::sqrt(p.x() * p.x() + p.y() * p.y());
        float theta = std::atan2(p.y(), p.x()); 
        if (theta < 0) theta += 2.0f * 3.14159265359f; 

        int zIdx = static_cast<int>((p.z() - minZ) / (maxZ - minZ) * (zSteps - 1));
        int aIdx = static_cast<int>((theta) / (2.0f * 3.14159265359f) * (angleSteps - 1));

        if (grid[zIdx][aIdx] < r) {
            grid[zIdx][aIdx] = r;
        }
    }

    for (int z = 0; z < zSteps; ++z) {
        for (int a = 0; a < angleSteps; ++a) {
            if (grid[z][a] < 0.0f) {
                int left = a - 1;
                while (left >= 0 && grid[z][left] < 0.0f) left--;
                int right = a + 1;
                while (right < angleSteps && grid[z][right] < 0.0f) right++;

                if (left >= 0 && right < angleSteps) {
                    float t = static_cast<float>(a - left) / (right - left);
                    grid[z][a] = grid[z][left] + t * (grid[z][right] - grid[z][left]);
                } else if (left >= 0) {
                    grid[z][a] = grid[z][left];
                } else if (right < angleSteps) {
                    grid[z][a] = grid[z][right];
                }
            }
        }
    }

    // Vertical fill
    for (int a = 0; a < angleSteps; ++a) {
        for (int z = 0; z < zSteps; ++z) {
            if (grid[z][a] < 0.0f) {
                int up = z - 1;
                while (up >= 0 && grid[up][a] < 0.0f) up--;
                int down = z + 1;
                while (down < zSteps && grid[down][a] < 0.0f) down++;

                if (up >= 0 && down < zSteps) {
                    float t = static_cast<float>(z - up) / (down - up);
                    grid[z][a] = grid[up][a] + t * (grid[down][a] - grid[up][a]);
                } else if (up >= 0) {
                    grid[z][a] = grid[up][a];
                } else if (down < zSteps) {
                    grid[z][a] = grid[down][a];
                }
            }
        }
    }


    QVector<QVector3D> triangles;
    triangles.reserve(zSteps * angleSteps * 6);

    auto getCartesian = [&](int zIdx, int aIdx) -> QVector3D {
        float r = grid[zIdx][aIdx];
        if (r < 0.0f) r = 0.0f; 
        float theta = static_cast<float>(aIdx) / (angleSteps - 1) * (2.0f * 3.14159265359f);
        float z = minZ + static_cast<float>(zIdx) / (zSteps - 1) * (maxZ - minZ);
        return QVector3D(r * std::cos(theta), r * std::sin(theta), z);
    };

    for (int z = 0; z < zSteps - 1; ++z) {
        for (int a = 0; a < angleSteps - 1; ++a) {
            if (grid[z][a] < 0.0f || grid[z+1][a] < 0.0f || grid[z][a+1] < 0.0f || grid[z+1][a+1] < 0.0f)
                continue;

            QVector3D p1 = getCartesian(z, a);
            QVector3D p2 = getCartesian(z + 1, a);
            QVector3D p3 = getCartesian(z, a + 1);
            QVector3D p4 = getCartesian(z + 1, a + 1);

            triangles.push_back(p1);
            triangles.push_back(p2);
            triangles.push_back(p3);
            triangles.push_back(p3);
            triangles.push_back(p2);
            triangles.push_back(p4);
        }
    }

    return triangles;
}

} // namespace core

