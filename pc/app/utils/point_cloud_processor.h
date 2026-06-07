#pragma once

#include <QVector>
#include <QVector3D>

namespace core {

class PointCloudProcessor {
public:
    // Remove points outside the given cylindrical radius (relative to X=0, Y=0)
    static QVector<QVector3D> filterCylindrical(const QVector<QVector3D>& input, float radiusMm, float minZ = -1000.0f, float maxZ = 1000.0f);

    // Remove points based on a list of indices
    static QVector<QVector3D> removePoints(const QVector<QVector3D>& input, const QVector<int>& indicesToRemove);
};

} // namespace core
