// ufo.cpp

#include "ufo.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

// ===================================================================
// PUBLIC: build a cylinder SimpleMeshData with a pre-transform
// ===================================================================
SimpleMeshData make_cylinder(
    bool        aCapped,
    std::size_t aSubdivs,
    Vec3f       aColor,
    Mat44f      aPreTransform,
    Vec3f       Ns,
    Vec3f       Ka,
    Vec3f       Kd,
    Vec3f       Ke,
    Vec3f       Ks
)
{
    SimpleMeshData mesh;

    std::vector<Vec3f> localPositions;
    std::vector<Vec3f> localNormals;

    // Unit cylinder in local space: radius 0.5, height 1 (-0.5 .. +0.5 in Y)
    float const radius = 0.5f;
    float const y0     = -0.5f;
    float const y1     =  0.5f;

    std::size_t slices = (aSubdivs < 3 ? 3 : aSubdivs);
    float const twoPi  = 2.0f * std::numbers::pi_v<float>;

    float shininess = Ns.x;

    // --- Sides ---
    for (std::size_t i = 0; i < slices; ++i)
    {
        float a0 = twoPi * float(i)       / float(slices);
        float a1 = twoPi * float(i + 1u)  / float(slices);

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        Vec3f p00{ radius * c0, y0, radius * s0 };
        Vec3f p01{ radius * c1, y0, radius * s1 };
        Vec3f p10{ radius * c0, y1, radius * s0 };
        Vec3f p11{ radius * c1, y1, radius * s1 };

        Vec3f n0{ c0, 0.0f, s0 };
        Vec3f n1{ c1, 0.0f, s1 };
        Vec3f nAvg{
            0.5f * (n0.x + n1.x),
            0.5f * (n0.y + n1.y),
            0.5f * (n0.z + n1.z)
        };
        nAvg = normalize(nAvg);

        // Triangle 1
        localPositions.push_back(p00);
        localNormals.push_back(nAvg);

        localPositions.push_back(p10);
        localNormals.push_back(nAvg);

        localPositions.push_back(p11);
        localNormals.push_back(nAvg);

        // Triangle 2
        localPositions.push_back(p00);
        localNormals.push_back(nAvg);

        localPositions.push_back(p11);
        localNormals.push_back(nAvg);

        localPositions.push_back(p01);
        localNormals.push_back(nAvg);
    }

    // --- Caps (optional) ---
    if (aCapped)
    {
        // Bottom cap
        Vec3f centerB{ 0.0f, y0, 0.0f };
        Vec3f nB{ 0.0f, -1.0f, 0.0f };

        for (std::size_t i = 0; i < slices; ++i)
        {
            float a0 = twoPi * float(i)       / float(slices);
            float a1 = twoPi * float(i + 1u)  / float(slices);

            Vec3f p0{ radius * std::cos(a0), y0, radius * std::sin(a0) };
            Vec3f p1{ radius * std::cos(a1), y0, radius * std::sin(a1) };

            // center, p1, p0 (CCW when seen from below)
            localPositions.push_back(centerB);
            localNormals.push_back(nB);

            localPositions.push_back(p1);
            localNormals.push_back(nB);

            localPositions.push_back(p0);
            localNormals.push_back(nB);
        }

        // Top cap
        Vec3f centerT{ 0.0f, y1, 0.0f };
        Vec3f nT{ 0.0f, 1.0f, 0.0f };

        for (std::size_t i = 0; i < slices; ++i)
        {
            float a0 = twoPi * float(i)       / float(slices);
            float a1 = twoPi * float(i + 1u)  / float(slices);

            Vec3f p0{ radius * std::cos(a0), y1, radius * std::sin(a0) };
            Vec3f p1{ radius * std::cos(a1), y1, radius * std::sin(a1) };

            // center, p0, p1 (CCW when seen from above)
            localPositions.push_back(centerT);
            localNormals.push_back(nT);

            localPositions.push_back(p0);
            localNormals.push_back(nT);

            localPositions.push_back(p1);
            localNormals.push_back(nT);
        }
    }

    // --- Apply pre-transform and fill SimpleMeshData ---
    mesh.positions.reserve(localPositions.size());
    mesh.normals.reserve(localNormals.size());
    mesh.colors.reserve(localPositions.size());
    mesh.Ka.reserve(localPositions.size());
    mesh.Kd.reserve(localPositions.size());
    mesh.Ke.reserve(localPositions.size());
    mesh.Ks.reserve(localPositions.size());
    mesh.Ns.reserve(localPositions.size());

    for (std::size_t i = 0; i < localPositions.size(); ++i)
    {
        Vec3f p = localPositions[i];
        Vec3f n = localNormals[i];

        // Position: w = 1
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f tp = aPreTransform * p4;
        if (tp.w != 0.f)
        {
            tp.x /= tp.w;
            tp.y /= tp.w;
            tp.z /= tp.w;
        }
        Vec3f worldPos{ tp.x, tp.y, tp.z };

        // Normal: w = 0, ignore translation
        Vec4f n4{ n.x, n.y, n.z, 0.f };
        Vec4f tn = aPreTransform * n4;
        Vec3f worldNormal{ tn.x, tn.y, tn.z };
        worldNormal = normalize(worldNormal);

        mesh.positions.push_back(worldPos);
        mesh.normals.push_back(worldNormal);

        mesh.colors.push_back(aColor);
        mesh.Ka.push_back(Ka);
        mesh.Kd.push_back(Kd);
        mesh.Ke.push_back(Ke);
        mesh.Ks.push_back(Ks);
        mesh.Ns.push_back(shininess);
    }

    return mesh;
}

// ===================================================================
// PUBLIC: build a cone SimpleMeshData with a pre-transform
// ===================================================================
SimpleMeshData make_cone(
    bool        aCapped,
    std::size_t aSubdivs,
    Vec3f       aColor,
    Mat44f      aPreTransform,
    Vec3f       Ns,
    Vec3f       Ka,
    Vec3f       Kd,
    Vec3f       Ke,
    Vec3f       Ks
)
{
    SimpleMeshData mesh;

    std::vector<Vec3f> localPositions;
    std::vector<Vec3f> localNormals;

    // Unit cone in local space: base radius 0.5 at y = -0.5, tip at y = +0.5
    float const radius = 0.5f;
    float const yBase  = -0.5f;
    float const yTip   =  0.5f;

    Vec3f tip{ 0.f, yTip, 0.f };

    std::size_t slices = (aSubdivs < 3 ? 3 : aSubdivs);
    float const twoPi  = 2.f * std::numbers::pi_v<float>;

    float shininess = Ns.x;

    // --- Side ---
    for (std::size_t i = 0; i < slices; ++i)
    {
        float a0 = twoPi * float(i)       / float(slices);
        float a1 = twoPi * float(i + 1u)  / float(slices);

        Vec3f p0{
            radius * std::cos(a0),
            yBase,
            radius * std::sin(a0)
        };
        Vec3f p1{
            radius * std::cos(a1),
            yBase,
            radius * std::sin(a1)
        };

        Vec3f e1 = p1 - p0;
        Vec3f e2 = tip - p0;
        Vec3f n  = normalize(cross(e2, e1));

        // Triangle: p0 -> tip -> p1
        localPositions.push_back(p0);
        localNormals.push_back(n);

        localPositions.push_back(tip);
        localNormals.push_back(n);

        localPositions.push_back(p1);
        localNormals.push_back(n);
    }

    // --- Base cap (optional) ---
    if (aCapped)
    {
        Vec3f centerB{ 0.f, yBase, 0.f };
        Vec3f nB{ 0.f, -1.f, 0.f };

        for (std::size_t i = 0; i < slices; ++i)
        {
            float a0 = twoPi * float(i)       / float(slices);
            float a1 = twoPi * float(i + 1u)  / float(slices);

            Vec3f p0{
                radius * std::cos(a0),
                yBase,
                radius * std::sin(a0)
            };
            Vec3f p1{
                radius * std::cos(a1),
                yBase,
                radius * std::sin(a1)
            };

            // center, p1, p0 (CCW when looking from below)
            localPositions.push_back(centerB);
            localNormals.push_back(nB);

            localPositions.push_back(p0);
            localNormals.push_back(nB);

            localPositions.push_back(p1);
            localNormals.push_back(nB);
        }
    }

    // --- Apply pre-transform and fill SimpleMeshData ---
    mesh.positions.reserve(localPositions.size());
    mesh.normals.reserve(localNormals.size());
    mesh.colors.reserve(localPositions.size());
    mesh.Ka.reserve(localPositions.size());
    mesh.Kd.reserve(localPositions.size());
    mesh.Ke.reserve(localPositions.size());
    mesh.Ks.reserve(localPositions.size());
    mesh.Ns.reserve(localPositions.size());

    for (std::size_t i = 0; i < localPositions.size(); ++i)
    {
        Vec3f p = localPositions[i];
        Vec3f n = localNormals[i];

        // position (w = 1)
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f tp = aPreTransform * p4;
        if (tp.w != 0.f)
        {
            tp.x /= tp.w;
            tp.y /= tp.w;
            tp.z /= tp.w;
        }
        Vec3f worldPos{ tp.x, tp.y, tp.z };

        // normal (w = 0)
        Vec4f n4{ n.x, n.y, n.z, 0.f };
        Vec4f tn = aPreTransform * n4;
        Vec3f worldNormal{ tn.x, tn.y, tn.z };
        worldNormal = normalize(worldNormal);

        mesh.positions.push_back(worldPos);
        mesh.normals.push_back(worldNormal);

        mesh.colors.push_back(aColor);
        mesh.Ka.push_back(Ka);
        mesh.Kd.push_back(Kd);
        mesh.Ke.push_back(Ke);
        mesh.Ks.push_back(Ks);
        mesh.Ns.push_back(shininess);
    }

    return mesh;
}
    SimpleMeshData make_fin(
    bool        aCapped,
    std::size_t aSubdivs,
    Vec3f       aColor,
    Mat44f      aPreTransform,
    Vec3f       Ns,
    Vec3f       Ka,
    Vec3f       Kd,
    Vec3f       Ke,
    Vec3f       Ks
)
{
    SimpleMeshData mesh;

    std::vector<Vec3f> localPositions;
    std::vector<Vec3f> localNormals;

    // Local fin: right triangle extruded in Z
    // Base along +X, height along +Y.
    // float halfT = 0.5f;  // base thickness in local space (we'll scale Z later)

    // Front (z = +halfT)
    Vec3f p0f{ 0.f, 0.f,  0.f };  // root
    Vec3f p1f{ 1.f, 0.f,  0.f };  // base tip
    Vec3f p2f{ 0.f, 1.f,  0.f };  // top

    // Back (z = -halfT)
    Vec3f p0b{ 0.f, 0.f, 0.f };
    Vec3f p1b{ 1.f, 0.f, 0.f };
    Vec3f p2b{ 0.f, 1.f, 0.f };

    Vec3f nFront{ 0.f, 0.f,  1.f };
    Vec3f nBack { 0.f, 0.f, -1.f };

    // --- Front face ---
    localPositions.push_back(p0f); localNormals.push_back(nFront);
    localPositions.push_back(p2f); localNormals.push_back(nFront);
    localPositions.push_back(p1f); localNormals.push_back(nFront);

    // --- Back face ---
    localPositions.push_back(p0b); localNormals.push_back(nBack);
    localPositions.push_back(p1b); localNormals.push_back(nBack);
    localPositions.push_back(p2b); localNormals.push_back(nBack);

    // --- Side: base edge (p0-p1) ---
    Vec3f nBase = normalize(Vec3f{ 0.f, -1.f, 0.f });  // pointing down
    localPositions.push_back(p0b); localNormals.push_back(nBase);
    localPositions.push_back(p1b); localNormals.push_back(nBase);
    localPositions.push_back(p1f); localNormals.push_back(nBase);

    localPositions.push_back(p0b); localNormals.push_back(nBase);
    localPositions.push_back(p1f); localNormals.push_back(nBase);
    localPositions.push_back(p0f); localNormals.push_back(nBase);

    // --- Side: vertical edge (p0-p2) ---
    Vec3f nVert = normalize(Vec3f{ -1.f, 0.f, 0.f }); // pointing -X
    localPositions.push_back(p0b); localNormals.push_back(nVert);
    localPositions.push_back(p2f); localNormals.push_back(nVert);
    localPositions.push_back(p2b); localNormals.push_back(nVert);

    localPositions.push_back(p0b); localNormals.push_back(nVert);
    localPositions.push_back(p0f); localNormals.push_back(nVert);
    localPositions.push_back(p2f); localNormals.push_back(nVert);

    // --- Side: hypotenuse edge (p1-p2) ---
    Vec3f edge = normalize(p2f - p1f);
    Vec3f nHyp = normalize(Vec3f{ edge.y, -edge.x, 0.f }); // 2D perp
    localPositions.push_back(p1b); localNormals.push_back(nHyp);
    localPositions.push_back(p2f); localNormals.push_back(nHyp);
    localPositions.push_back(p2b); localNormals.push_back(nHyp);

    localPositions.push_back(p1b); localNormals.push_back(nHyp);
    localPositions.push_back(p1f); localNormals.push_back(nHyp);
    localPositions.push_back(p2f); localNormals.push_back(nHyp);

    // ---- Fill SimpleMeshData (same pattern as cylinder/cone) ----
    mesh.positions.reserve(localPositions.size());
    mesh.normals.reserve(localNormals.size());
    mesh.colors.reserve(localPositions.size());
    mesh.Ka.reserve(localPositions.size());
    mesh.Kd.reserve(localPositions.size());
    mesh.Ke.reserve(localPositions.size());
    mesh.Ks.reserve(localPositions.size());
    mesh.Ns.reserve(localPositions.size());

    float shininess = Ns.x;

    for (std::size_t i = 0; i < localPositions.size(); ++i)
    {
        Vec3f p = localPositions[i];
        Vec3f n = localNormals[i];

        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f tp = aPreTransform * p4;
        if (tp.w != 0.f)
        {
            tp.x /= tp.w;
            tp.y /= tp.w;
            tp.z /= tp.w;
        }
        Vec3f worldPos{ tp.x, tp.y, tp.z };

        Vec4f n4{ n.x, n.y, n.z, 0.f };
        Vec4f tn = aPreTransform * n4;
        Vec3f worldNormal{ tn.x, tn.y, tn.z };
        worldNormal = normalize(worldNormal);

        mesh.positions.push_back(worldPos);
        mesh.normals.push_back(worldNormal);

        mesh.colors.push_back(aColor);
        mesh.Ka.push_back(Ka);
        mesh.Kd.push_back(Kd);
        mesh.Ke.push_back(Ke);
        mesh.Ks.push_back(Ks);
        mesh.Ns.push_back(shininess);
    }

    return mesh;
}
    // Build a unit cube centred at the origin, then pre-transform it.
// Size = 1 in each axis before scaling.
SimpleMeshData make_cube(
    bool        aCapped,
    std::size_t aSubdivs,
    Vec3f       aColor,
    Mat44f      aPreTransform,
    Vec3f       Ns,
    Vec3f       Ka,
    Vec3f       Kd,
    Vec3f       Ke,
    Vec3f       Ks
)
{
    SimpleMeshData mesh;

    std::vector<Vec3f> localPos;
    std::vector<Vec3f> localNorm;

    // --- Local vertices (cube centred at origin, edge length 1) ---
    Vec3f v000{ -0.5f, -0.5f, -0.5f };
    Vec3f v001{ -0.5f, -0.5f,  0.5f };
    Vec3f v010{ -0.5f,  0.5f, -0.5f };
    Vec3f v011{ -0.5f,  0.5f,  0.5f };
    Vec3f v100{  0.5f, -0.5f, -0.5f };
    Vec3f v101{  0.5f, -0.5f,  0.5f };
    Vec3f v110{  0.5f,  0.5f, -0.5f };
    Vec3f v111{  0.5f,  0.5f,  0.5f };

    auto add_face = [&](const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& n)
    {
        // a,b,c are CCW when viewed from outside face with normal n
        localPos.push_back(a); localNorm.push_back(n);
        localPos.push_back(b); localNorm.push_back(n);
        localPos.push_back(c); localNorm.push_back(n);
    };

    // +X face
    add_face(v100, v111, v101, Vec3f{ 1.f, 0.f, 0.f });
    add_face(v100, v110, v111, Vec3f{ 1.f, 0.f, 0.f });

    // -X face
    add_face(v000, v011, v010, Vec3f{ -1.f, 0.f, 0.f });
    add_face(v000, v001, v011, Vec3f{ -1.f, 0.f, 0.f });

    // +Y face
    add_face(v010, v111, v110, Vec3f{ 0.f, 1.f, 0.f });
    add_face(v010, v011, v111, Vec3f{ 0.f, 1.f, 0.f });

    // -Y face
    add_face(v000, v101, v001, Vec3f{ 0.f,-1.f, 0.f });
    add_face(v000, v100, v101, Vec3f{ 0.f,-1.f, 0.f });

    // +Z face
    add_face(v001, v111, v011, Vec3f{ 0.f, 0.f, 1.f });
    add_face(v001, v101, v111, Vec3f{ 0.f, 0.f, 1.f });

    // -Z face
    add_face(v000, v110, v100, Vec3f{ 0.f, 0.f,-1.f });
    add_face(v000, v010, v110, Vec3f{ 0.f, 0.f,-1.f });

    // --- Apply pre-transform & fill SimpleMeshData ---
    for (std::size_t i = 0; i < localPos.size(); ++i)
    {
        Vec3f p = localPos[i];
        Vec3f n = localNorm[i];

        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f n4{ n.x, n.y, n.z, 0.f };

        Vec4f tp4 = aPreTransform * p4;
        Vec4f tn4 = aPreTransform * n4;

        Vec3f tp{ tp4.x, tp4.y, tp4.z };
        Vec3f tn = normalize(Vec3f{ tn4.x, tn4.y, tn4.z });

        mesh.positions.push_back(tp);
        mesh.normals.push_back(tn);

        // follow same pattern as your other make_* functions
        mesh.colors.push_back(aColor);
        mesh.Ka.push_back(Ka);
        mesh.Kd.push_back(Kd);
        mesh.Ke.push_back(Ke);
        mesh.Ks.push_back(Ks);
        mesh.Ns.push_back(Ns.x);
    }

    return mesh;
}
// ===================================================================