#include "stl_loader.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <iostream>

namespace sim {

static void updateBBox(Mesh& m, const Vec3f& v) {
    if (v.x < m.bbMin.x) m.bbMin.x = v.x;
    if (v.y < m.bbMin.y) m.bbMin.y = v.y;
    if (v.z < m.bbMin.z) m.bbMin.z = v.z;
    if (v.x > m.bbMax.x) m.bbMax.x = v.x;
    if (v.y > m.bbMax.y) m.bbMax.y = v.y;
    if (v.z > m.bbMax.z) m.bbMax.z = v.z;
}

// --- Binary STL Implementation ------------------------------------
template<typename StreamType>
static Mesh loadBinaryInternal(StreamType& f) {
    f.seekg(0, std::ios::end);
    std::streamsize fileSize = f.tellg();
    f.seekg(0, std::ios::beg);
    if (fileSize < 84) return {};

    char header[80];
    f.read(header, 80);
    uint32_t count = 0;
    f.read(reinterpret_cast<char*>(&count), 4);

    Mesh mesh;
    mesh.triangles.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        if (f.eof()) break;
        float buf[12]; 
        f.read(reinterpret_cast<char*>(buf), 48);
        uint16_t attr; f.read(reinterpret_cast<char*>(&attr), 2);

        Triangle tri;
        for (int j = 0; j < 3; ++j) {
            tri.v[j] = { buf[3 + j*3], buf[4 + j*3], buf[5 + j*3] };
            updateBBox(mesh, tri.v[j]);
        }
        mesh.triangles.push_back(tri);
    }
    return mesh;
}

// --- ASCII STL Implementation -------------------------------------
template<typename StreamType>
static Mesh loadAsciiInternal(StreamType& f) {
    Mesh mesh;
    std::string token;
    Triangle tri;
    int vIdx = 0;
    f.seekg(0, std::ios::beg);
    while (f >> token) {
        if (token == "vertex") {
            float x, y, z;
            if (!(f >> x >> y >> z)) break;
            tri.v[vIdx] = {x, y, z};
            updateBBox(mesh, tri.v[vIdx]);
            if (++vIdx == 3) {
                mesh.triangles.push_back(tri);
                vIdx = 0;
            }
        }
    }
    return mesh;
}

template <typename StreamType>
static Mesh loadGeneric(StreamType& f) {
    if (!f.is_open()) return {};
    f.seekg(0, std::ios::beg);
    char header[6] = {};
    f.read(header, 5);
    bool startsWithSolid = (std::strncmp(header, "solid", 5) == 0);
    
    f.seekg(0, std::ios::end);
    std::streamsize size = f.tellg();
    f.seekg(80, std::ios::beg);
    uint32_t count = 0;
    f.read(reinterpret_cast<char*>(&count), 4);
    bool looksLikeBinary = (size == static_cast<std::streamsize>(84 + count * 50));
    
    Mesh m;
    if (startsWithSolid && !looksLikeBinary) {
        m = loadAsciiInternal(f);
    } else {
        f.seekg(0, std::ios::beg);
        m = loadBinaryInternal(f);
    }

    if (!m.triangles.empty()) {
        float xShift = -(m.bbMin.x + m.bbMax.x) * 0.5f;
        float yShift = -(m.bbMin.y + m.bbMax.y) * 0.5f;
        float zShift = -m.bbMin.z;
        for (auto& tri : m.triangles) {
            for (auto& v : tri.v) {
                v.x += xShift; v.y += yShift; v.z += zShift;
            }
        }
        m.bbMin.x += xShift; m.bbMax.x += xShift;
        m.bbMin.y += yShift; m.bbMax.y += yShift;
        m.bbMin.z = 0.0f; m.bbMax.z += zShift;
    }
    return m;
}

Mesh loadSTL(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return loadGeneric(f);
}
Mesh loadSTL(const std::wstring& path) {
    std::ifstream f(path, std::ios::binary);
    return loadGeneric(f);
}

} // namespace sim