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
    // Hizalama Düzeltmesi: -theta_rad - PI/2 (90 derece ofset)
    const float PI = 3.1415926535f;
    float slice_angle = -theta_rad - (PI / 2.0f);
    float cosA = std::cos(slice_angle);
    float sinA = std::sin(slice_angle);

    std::map<int, float> zMap;
    const float Z_SCALE = 5.0f; 

    for (const auto& tri : mesh.triangles) {
        Vec3f rv[3] = {
            rotateZ(tri.v[0], cosA, sinA),
            rotateZ(tri.v[1], cosA, sinA),
            rotateZ(tri.v[2], cosA, sinA)
        };

        // Z-Filtre (0-22mm gibi bir limit yoksa tamamını al)
        // Kullanıcı 0-22mm arasını görmek istiyorsa burada bırakabiliriz
        // ama genel olsun diye tüm mesh'i tarayalım.
        
        Vec3f hits[3]; int n = 0; Vec3f h;
        if (intersectEdgeY(rv[0], rv[1], 0.0f, h)) hits[n++] = h;
        if (intersectEdgeY(rv[1], rv[2], 0.0f, h)) hits[n++] = h;
        if (intersectEdgeY(rv[2], rv[0], 0.0f, h)) hits[n++] = h;

        if (n >= 2) {
            float z0 = hits[0].z, z1 = hits[1].z;
            float x0 = hits[0].x, x1 = hits[1].x;
            float startZ = std::min(z0, z1);
            float endZ = std::max(z0, z1);
            for (float z = std::ceil(startZ * Z_SCALE) / Z_SCALE; z <= endZ; z += 0.2f) {
                int zIdx = (int)(z * Z_SCALE);
                float t = (std::abs(z1 - z0) < 1e-7f) ? 0.0f : (z - z0) / (z1 - z0);
                float x = x0 + (x1 - x0) * t;
                if (zMap.find(zIdx) == zMap.end() || x > zMap[zIdx]) zMap[zIdx] = x;
            }
        }
    }

    std::vector<ProfilePoint> pts;
    for (auto const& [zIdx, x] : zMap) {
        // x = Yarıçap (r) as it's already centered
        pts.push_back({ x, (float)zIdx / Z_SCALE });
    }
    return pts;
}

} // namespace sim