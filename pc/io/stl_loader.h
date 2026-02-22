#pragma once
#include <vector>
#include <string>
#include <array>

namespace sim {

struct Vec3f { float x, y, z; };
struct Triangle { Vec3f v[3]; };

struct Mesh {
    std::vector<Triangle> triangles;
    // Bounding box
    Vec3f bbMin{1e9f,1e9f,1e9f}, bbMax{-1e9f,-1e9f,-1e9f};

    float centerZ()  const { return (bbMin.z + bbMax.z) * 0.5f; }
    float extentZ()  const { return bbMax.z - bbMin.z; }
    float extentXY() const {
        float dx = bbMax.x - bbMin.x;
        float dy = bbMax.y - bbMin.y;
        return dx > dy ? dx : dy;
    }
    bool empty() const { return triangles.empty(); }
};

// UTF-8 veya local ANSI yol ile yukle
Mesh loadSTL(const std::string& path);

// Windows icin wide-path dekteli yukleme
Mesh loadSTL(const std::wstring& path);

} // namespace sim
