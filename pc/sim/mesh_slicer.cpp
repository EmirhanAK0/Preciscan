#include "mesh_slicer.h"
#include <cmath>
#include <algorithm>
#include <map>

namespace sim {

static Vec3f rotateZ(const Vec3f& v, float cosA, float sinA) {
    return { v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA, v.z };
}

static bool intersectEdgeX(const Vec3f& a, const Vec3f& b, float targetX, Vec3f& hit) {
    if ((a.x < targetX && b.x < targetX) || (a.x > targetX && b.x > targetX)) return false;
    if (std::abs(a.x - b.x) < 1e-9f) return false;
    float t = (targetX - a.x) / (b.x - a.x);
    hit.x = targetX;
    hit.y = a.y + (b.y - a.y) * t;
    hit.z = a.z + (b.z - a.z) * t;
    return true;
}

std::vector<ProfilePoint> sliceAtAngle(const Mesh& mesh,
                                       float theta_rad,
                                       const SliceParams& params)
{
    float cosA = std::cos(-theta_rad);
    float sinA = std::sin(-theta_rad);

    std::map<int, float> zMap;
    const float Z_SCALE = 5.0f; // 0.2mm precision

    for (const auto& tri : mesh.triangles) {
        Vec3f rv[3] = { rotateZ(tri.v[0], cosA, sinA), rotateZ(tri.v[1], cosA, sinA), rotateZ(tri.v[2], cosA, sinA) };
        float tminZ = std::min({rv[0].z, rv[1].z, rv[2].z});
        float tmaxZ = std::max({rv[0].z, rv[1].z, rv[2].z});
        // 22mm dikey sensor simülasyonu
        if (tmaxZ < 0.0f || tminZ > 22.0f) continue;

        Vec3f hits[3]; int n = 0; Vec3f h;
        if (intersectEdgeX(rv[0], rv[1], 0.0f, h)) hits[n++] = h;
        if (intersectEdgeX(rv[1], rv[2], 0.0f, h)) hits[n++] = h;
        if (intersectEdgeX(rv[2], rv[0], 0.0f, h)) hits[n++] = h;

        if (n >= 2) {
            float z0 = hits[0].z, z1 = hits[1].z;
            float y0 = hits[0].y, y1 = hits[1].y;
            float startZ = std::max(0.0f, std::min(z0, z1));
            float endZ = std::min(22.0f, std::max(z0, z1));
            for (float z = std::ceil(startZ * Z_SCALE) / Z_SCALE; z <= endZ; z += 0.2f) {
                int zIdx = (int)(z * Z_SCALE);
                float t = (std::abs(z1 - z0) < 1e-7f) ? 0.0f : (z - z0) / (z1 - z0);
                float y = y0 + (y1 - y0) * t;
                // Look from +Y
                if (zMap.find(zIdx) == zMap.end() || y > zMap[zIdx]) zMap[zIdx] = y;
            }
        }
    }

    std::vector<ProfilePoint> pts;
    for (auto const& [zIdx, y] : zMap) {
        float dist = params.D_offset_mm - y;
        // ProfilePoint: r = distance, z = height
        pts.push_back({ dist, (float)zIdx / Z_SCALE }); 
    }
    return pts;
}

} // namespace sim