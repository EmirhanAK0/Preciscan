#pragma once
#include "../io/stl_loader.h"
#include <vector>

namespace sim {

struct ProfilePoint {
    float r;  // Mesafe veya Yaricap (mm)
    float z;  // Yukseklik veya Z (mm)
};

struct SliceParams {
    float D_offset_mm = 66.0f;
    float deg_per_step = 1.0f;
    float rps = 10.0f;
};

std::vector<ProfilePoint> sliceAtAngle(const Mesh& mesh,
                                       float theta_rad,
                                       const SliceParams& params);

} // namespace sim