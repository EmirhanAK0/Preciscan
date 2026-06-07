#pragma once

#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include "../../third_party/nanoflann.hpp"

namespace core {

// Nanoflann Adapter for QVector<QVector3D>
struct PointCloudAdapter {
    const QVector<QVector3D>& points;
    PointCloudAdapter(const QVector<QVector3D>& pts) : points(pts) {}

    inline size_t kdtree_get_point_count() const { return points.size(); }

    inline float kdtree_get_pt(const size_t idx, const size_t dim) const {
        if (dim == 0) return points[idx].x();
        else if (dim == 1) return points[idx].y();
        else return points[idx].z();
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, PointCloudAdapter>,
    PointCloudAdapter, 3> PointCloudKDTree;

class PointCloudProcessor {
public:
    // Remove points outside the given cylindrical radius (relative to X=0, Y=0)
    static QVector<QVector3D> filterCylindrical(const QVector<QVector3D>& input, float radiusMm, float minZ = -1000.0f, float maxZ = 1000.0f);

    // Remove points based on a list of indices
    static QVector<QVector3D> removePoints(const QVector<QVector3D>& input, const QVector<int>& indicesToRemove);

    // Yeni Filtreler
    static QVector<QVector3D> filterStatisticalOutlier(const QVector<QVector3D>& input, int meanK = 50, float stdDevMulThresh = 1.0f);
    static QVector<QVector3D> filterRadiusOutlier(const QVector<QVector3D>& input, float radius = 2.0f, int minNeighbors = 10);
    static QVector<QVector3D> filterVoxelGrid(const QVector<QVector3D>& input, float leafSize = 1.0f);

    // Kaba (Coarse) Hizalama
    static QVector<QVector3D> transformCloud(const QVector<QVector3D>& input, const QMatrix4x4& transform);

    // Hassas Hizalama (ICP - Iterative Closest Point)
    static QMatrix4x4 calculateICP(const QVector<QVector3D>& source, const QVector<QVector3D>& target, int maxIterations = 50, float tolerance = 1e-5f);
};

} // namespace core
