#include "point_cloud_processor.h"
#include <vector>

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

} // namespace core
