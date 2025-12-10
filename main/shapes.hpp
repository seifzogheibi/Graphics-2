// ufo.hpp
#pragma once
#include <vector>
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "simple_mesh.hpp"


SimpleMeshData make_cylinder(
bool aCapped = true,
std::size_t aSubdivs = 16,
Vec3f aColor = { 1.f, 1.f, 1.f },
Mat44f aPreTransform = kIdentity44f,
Vec3f Ns = { 32.f, 0.f, 0.f },
Vec3f Ka = { 0.2f, 0.2f, 0.2f },
Vec3f Kd = { 0.8f, 0.8f, 0.8f },
Vec3f Ke = { 0.f, 0.f, 0.f },
Vec3f Ks = { 0.5f, 0.5f, 0.5f }
);

SimpleMeshData make_cone(
    bool        aCapped      = true,
    std::size_t aSubdivs     = 16,
    Vec3f       aColor       = { 1.f, 1.f, 1.f },
    Mat44f      aPreTransform = kIdentity44f,
    Vec3f Ns = { 32.f, 0.f, 0.f },
Vec3f Ka = { 0.2f, 0.2f, 0.2f },
Vec3f Kd = { 0.8f, 0.8f, 0.8f },
Vec3f Ke = { 0.f, 0.f, 0.f },
Vec3f Ks = { 0.5f, 0.5f, 0.5f }
);

SimpleMeshData make_fin(
    bool        aCapped      = true,
    std::size_t aSubdivs     = 16,
    Vec3f       aColor      = { 1.f, 1.f, 1.f },
    Mat44f      aPreTransform = kIdentity44f,
    Vec3f Ns = { 32.f, 0.f, 0.f },
Vec3f Ka = { 0.2f, 0.2f, 0.2f },
Vec3f Kd = { 0.8f, 0.8f, 0.8f },
Vec3f Ke = { 0.f, 0.f, 0.f },
Vec3f Ks = { 0.5f, 0.5f, 0.5f }
);

SimpleMeshData make_cube(
    bool        aCapped      = true,
    std::size_t aSubdivs     = 16,
    Vec3f       aColor      = { 1.f, 1.f, 1.f },
    Mat44f      aPreTransform = kIdentity44f,
    Vec3f Ns = { 32.f, 0.f, 0.f },
    Vec3f Ka = { 0.2f, 0.2f, 0.2f },
    Vec3f Kd = { 0.8f, 0.8f, 0.8f },
    Vec3f Ke = { 0.f, 0.f, 0.f },
    Vec3f Ks = { 0.5f, 0.5f, 0.5f }
);