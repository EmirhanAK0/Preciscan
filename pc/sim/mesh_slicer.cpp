#include "mesh_slicer.h"
#include <cmath>
#include <algorithm>
#include <map>

namespace sim {

static Vec3f rotateZ(const Vec3f& v, float cosA, float sinA) {
    return { v.x * cosA - v.y * sinA,
             v.x * sinA + v.y * cosA,
             v.z };
}

// Y=targetY duzlemi ile kesisim (X ekseni uzerinde tarama yapmak icin)
static bool intersectEdgeY(const Vec3f& a, const Vec3f& b, float targetY, Vec3f& hit) {
    if ((a.y < targetY && b.y < targetY) || (a.y > targetY && b.y > targetY)) return false;
    if (std::abs(a.y - b.y) < 1e-9f) return false;
    float t = (targetY - a.y) / (b.y - a.y);
    hit.x = a.x + (b.x - a.x) * t;
    hit.y = targetY;
    hit.z = a.z + (b.z - a.z) * t;
    return true;
}

std::vector<ProfilePoint> sliceAtAngle(const Mesh& mesh,
                                       float theta_rad,
                                       const SliceParams& params)
{
    // Vizualizerda theta=0 acisi +X yonundedir. 
    // Bu yuzden Y=0 duzlemi ile kesisim aliyoruz.
    float cosA = std::cos(-theta_rad);
    float sinA = std::sin(-theta_rad);

    std::map<int, float> zMap;
    const float Z_SCALE = 5.0f; // 0.2mm hassasiyet

    for (const auto& tri : mesh.triangles) {
        Vec3f rv[3] = {
            rotateZ(tri.v[0], cosA, sinA),
            rotateZ(tri.v[1], cosA, sinA),
            rotateZ(tri.v[2], cosA, sinA)
        };

        float tminZ = std::min({rv[0].z, rv[1].z, rv[2].z});
        float tmaxZ = std::max({rv[0].z, rv[1].z, rv[2].z});
        if (tmaxZ < 0.0f || tminZ > 22.0f) continue;

        // Y=0 duzlemi kesişimi (X ekseni uzerindeki yuk)
        Vec3f hits[3]; int n = 0;
        Vec3f h;
        if (intersectEdgeY(rv[0], rv[1], 0.0f, h)) hits[n++] = h;
        if (intersectEdgeY(rv[1], rv[2], 0.0f, h)) hits[n++] = h;
        if (intersectEdgeY(rv[2], rv[0], 0.0f, h)) hits[n++] = h;

        if (n >= 2) {
            float z0 = hits[0].z, z1 = hits[1].z;
            float x0 = hits[0].x, x1 = hits[1].x;

            float startZ = std::max(0.0f, std::min(z0, z1));
            float endZ = std::min(22.0f, std::max(z0, z1));

            for (float z = std::ceil(startZ * Z_SCALE) / Z_SCALE; z <= endZ; z += 0.2f) {
                int zIdx = (int)(z * Z_SCALE);
                float t = (std::abs(z1 - z0) < 1e-7f) ? 0.0f : (z - z0) / (z1 - z0);
                float x = x0 + (x1 - x0) * t;

                // Lazer +X yonunden bakar (Visualizer uyumu icin)
                // En buyuk X, lazere en yakindir.
                if (zMap.find(zIdx) == zMap.end() || x > zMap[zIdx]) {
                    zMap[zIdx] = x;
                }
            }
        }
    }

    std::vector<ProfilePoint> pts;
    pts.reserve(zMap.size());
    for (auto const& [zIdx, x] : zMap) {
        // Mesafe (d) = D_offset - x
        float dist = params.D_offset_mm - x;
        pts.push_back({ (float)zIdx / Z_SCALE, dist });
    }

    return pts;
}

} // namespace sim