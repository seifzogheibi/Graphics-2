// spaceship.cpp

#include "spaceship.hpp"
#include "shapes.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
// ===================================================================
// PUBLIC: build a cylinder SimpleMeshData with a pre-transform
// ===================================================================
SimpleMeshData make_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)
{
    std::vector<Vec3f> position;
    std::vector<Vec3f> normal;

    float prevY = std::cos( 0.f );
    float prevZ = std::sin( 0.f );
    

    for( std::size_t i = 0; i < aSubdivs; ++i )
    {
     float const angle = (i+1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
     float y = std::cos( angle );
     float z = std::sin( angle );
     Vec3f nAvg{
        0.f,
        0.5f * (prevY + y),
        0.5f * (prevZ + z)
    };
    nAvg = normalize( nAvg );

    // Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
    position.emplace_back(Vec3f{ 0.f, prevY, prevZ });
    position.emplace_back(Vec3f{ 0.f, y, z });
	position.emplace_back(Vec3f{ 1.f, prevY, prevZ });
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );

	position.emplace_back(Vec3f{ 0.f, y, z });
	position.emplace_back(Vec3f{ 1.f, y, z });
	position.emplace_back(Vec3f{ 1.f, prevY, prevZ });
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );

    prevY = y;
    prevZ = z;
    }

    // --- Caps (optional) ---
    if (aCapped)
    {
        prevY = std::cos(0.f);
        prevZ = std::sin(0.f);
        Vec3f centerB{ 0.f, 0.f, 0.f };
        Vec3f nB{ -1.f, 0.f, 0.f };
        Vec3f p0{ 0.f, prevY, prevZ };
        

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);
            Vec3f p1{ 0.f, y,z};

            // center, p1, p0
            position.emplace_back(centerB);
            position.emplace_back(p1);
            position.emplace_back(p0);

            normal.emplace_back(nB);
            normal.emplace_back(nB);
            normal.emplace_back(nB);

            prevY = y;
            prevZ = z;
        }
    }

    for( auto& p : position )
    {
    Vec4f p4{ p.x, p.y, p.z, 1.f };
    Vec4f t = aPreTransform * p4;
    t /= t.w;
    p = Vec3f{ t.x, t.y, t.z };
    }
    for( auto& n : normal )
    {
    Vec4f n4{ n.x, n.y, n.z, 0.f };
    Vec4f t = aPreTransform * n4;
    n = Vec3f{ t.x, t.y, t.z };
    n = normalize( n );
    }

    SimpleMeshData mesh;
    mesh.positions = std::move( position );
    mesh.normals = std::move( normal );
    mesh.colors.resize( mesh.positions.size(), aColor );
    mesh.Ka.resize( mesh.positions.size(), Ka );
    mesh.Kd.resize( mesh.positions.size(), Kd );
    mesh.Ke.resize( mesh.positions.size(), Ke );
    mesh.Ks.resize( mesh.positions.size(), Ks );
    mesh.Ns.resize( mesh.positions.size(), Ns );

    return mesh;
}
// ===================================================================
// PUBLIC: build a cone SimpleMeshData with a pre-transform
// ===================================================================
SimpleMeshData make_cone(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)
{

    std::vector<Vec3f> position;
    std::vector<Vec3f> normal;

    float prevY = std::cos( 0.f );
    float prevZ = std::sin( 0.f );

    Vec3f apex{ 1.f, 0.f, 0.f };
    

    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos(angle);
        float z = std::sin(angle);

        Vec3f p0{ 0.f, prevY, prevZ };
        Vec3f p1{ 0.f, y,     z     };

        // one triangle per segment: p0 -> p1 -> apex
        position.emplace_back(p0);
        position.emplace_back(p1);
        position.emplace_back(apex);

             Vec3f nm{
        0.f,
        0.5f * (prevY + y),
        0.5f * (prevZ + z)
    };
    nm = normalize( nm );


        normal.emplace_back(nm);
        normal.emplace_back(nm);
        normal.emplace_back(nm);

        prevY = y;
        prevZ = z;
    }

    // ========== BOTTOM CAP (optional) ==========
    if (aCapped)
    {
        prevY = std::cos(0.f);
        prevZ = std::sin(0.f);

        Vec3f centerB{ 0.f, 0.f, 0.f };
        Vec3f nB{ -1.f, 0.f, 0.f };

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);

            Vec3f p0{ 0.f, prevY, prevZ };
            Vec3f p1{ 0.f, y, z };

            // center, p1, p0
            position.emplace_back(centerB);
            position.emplace_back(p1);
            position.emplace_back(p0);
            normal.emplace_back(nB);
            normal.emplace_back(nB);
            normal.emplace_back(nB);

            prevY = y;
            prevZ = z;
        }
    }

    for( auto& p : position )
    {
    Vec4f p4{ p.x, p.y, p.z, 1.f };
    Vec4f t = aPreTransform * p4;
    t /= t.w;
    p = Vec3f{ t.x, t.y, t.z };
    }
    for( auto& n : normal )
    {
    Vec4f n4{ n.x, n.y, n.z, 0.f };
    Vec4f t = aPreTransform * n4;
    n = Vec3f{ t.x, t.y, t.z };
    n = normalize( n );
    }
    SimpleMeshData mesh;
    mesh.positions = std::move( position );
    mesh.normals = std::move( normal );
    mesh.colors.resize( mesh.positions.size(), aColor );
    mesh.Ka.resize( mesh.positions.size(), Ka );
    mesh.Kd.resize( mesh.positions.size(), Kd );
    mesh.Ke.resize( mesh.positions.size(), Ke );
    mesh.Ks.resize( mesh.positions.size(), Ks );
    mesh.Ns.resize( mesh.positions.size(), Ns );

    return mesh;
}

SimpleMeshData make_fin(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)

{
    std::vector<Vec3f> position;
    std::vector<Vec3f> normal;

    // ----- Local fin geometry -----
    // Right triangle extruded in Z.
    const float halfT = 0.5f;     // thickness/2 in local Z

    // Front (z = +halfT)
    Vec3f p0f{ 0.f, 0.f,  halfT };  // root
    Vec3f p2f{ 1.f, 0.f,  halfT };  // base tip
    Vec3f p1f{ 0.f, 1.f,  halfT };  // top

    // Back (z = -halfT)
    Vec3f p0b{ 0.f, 0.f, -halfT };
    Vec3f p2b{ 1.f, 0.f, -halfT };
    Vec3f p1b{ 0.f, 1.f, -halfT };

    // ----- Front face (p0f, p2f, p1f) -----
    {
        Vec3f e0 = p2f - p0f;
        Vec3f e1 = p1f - p0f;
        Vec3f nFront = normalize(cross(e0, e1)); // should be (0,0,1)

        position.emplace_back(p0f);
        position.emplace_back(p2f);
        position.emplace_back(p1f);
        normal.emplace_back(nFront);
        normal.emplace_back(nFront);
        normal.emplace_back(nFront);
    }

    // ----- Back face (p0b, p1b, p2b) -----
    {
        Vec3f e0 = p1b - p0b;
        Vec3f e1 = p2b - p0b;
        Vec3f nBack = normalize(cross(e0, e1));  // should be (0,0,-1)

        position.emplace_back(p0b);
        position.emplace_back(p1b);
        position.emplace_back(p2b);
        normal.emplace_back(nBack);
        normal.emplace_back(nBack);
        normal.emplace_back(nBack);
    }

    // ----- Side: base edge (p0–p2) -----
    {
        Vec3f v0 = p0b;
        Vec3f v1 = p2b;
        Vec3f v2 = p2f;
        Vec3f e0 = v1 - v0;
        Vec3f e1 = v2 - v0;
        Vec3f nBase = normalize(cross(e0, e1));   // roughly (0,-1,0)

        position.emplace_back(p0b);
        position.emplace_back(p2b);
        position.emplace_back(p2f);
        normal.emplace_back(nBase);
        normal.emplace_back(nBase);
        normal.emplace_back(nBase);

        position.emplace_back(p0b);
        position.emplace_back(p2f);
        position.emplace_back(p0f);
        normal.emplace_back(nBase);
        normal.emplace_back(nBase);
        normal.emplace_back(nBase);
    }

    // ----- Side: vertical edge (p0–p1) -----
    {
        Vec3f v0 = p0b;
        Vec3f v1 = p1f;
        Vec3f v2 = p1b;
        Vec3f e0 = v1 - v0;
        Vec3f e1 = v2 - v0;
        Vec3f nVert = normalize(cross(e0, e1));   // roughly (-1,0,0)

        position.emplace_back(p0b);
        position.emplace_back(p1f);
        position.emplace_back(p1b);
        normal.emplace_back(nVert);
        normal.emplace_back(nVert);
        normal.emplace_back(nVert);

        position.emplace_back(p0b);
        position.emplace_back(p0f);
        position.emplace_back(p1f);
        normal.emplace_back(nVert);
        normal.emplace_back(nVert);
        normal.emplace_back(nVert);
    }

    // ----- Side: hypotenuse edge (p2–p1) -----
    {
        Vec3f v0 = p2b;
        Vec3f v1 = p1b;
        Vec3f v2 = p1f;
        Vec3f e0 = v1 - v0;
        Vec3f e1 = v2 - v0;
        Vec3f nHyp = normalize(cross(e0, e1));

        position.emplace_back(p2b);
        position.emplace_back(p1b);
        position.emplace_back(p1f);
        normal.emplace_back(nHyp);
        normal.emplace_back(nHyp);
        normal.emplace_back(nHyp);

        position.emplace_back(p2b);
        position.emplace_back(p1f);
        position.emplace_back(p2f);
        normal.emplace_back(nHyp);
        normal.emplace_back(nHyp);
        normal.emplace_back(nHyp);
    }

    // ----- Apply pre-transform (same as cylinder/cone) -----
    for (auto& p : position)
    {
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f t = aPreTransform * p4;
        t /= t.w;
        p = Vec3f{ t.x, t.y, t.z };
    }

    for (auto& n : normal)
    {
        Vec4f n4{ n.x, n.y, n.z, 0.f };
        Vec4f t = aPreTransform * n4;
        Vec3f nn{ t.x, t.y, t.z };
        n = normalize(nn);
    }

    // ----- Fill SimpleMeshData (same pattern as cylinder/cone) -----
    SimpleMeshData mesh;
    mesh.positions = std::move( position );
    mesh.normals = std::move( normal );
    mesh.colors.resize( mesh.positions.size(), aColor );
    mesh.Ka.resize( mesh.positions.size(), Ka );
    mesh.Kd.resize( mesh.positions.size(), Kd );
    mesh.Ke.resize( mesh.positions.size(), Ke );
    mesh.Ks.resize( mesh.positions.size(), Ks );
    mesh.Ns.resize( mesh.positions.size(), Ns );

    return mesh;
}


    // Build a unit cube centred at the origin, then pre-transform it.
// Size = 1 in each axis before scaling.
SimpleMeshData make_cube(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)
{
    std::vector<Vec3f> position;
    std::vector<Vec3f> normal;

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

    // -Y face are CCW when viewed from outside face with normal n
        position.emplace_back(a); normal.emplace_back(n);
        position.emplace_back(b); normal.emplace_back(n);
        position.emplace_back(c); normal.emplace_back(n);
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
        for (auto& p : position)
    {
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f t = aPreTransform * p4;
        t /= t.w;
        p = Vec3f{ t.x, t.y, t.z };
    }

    for (auto& n : normal)
    {
        Vec4f n4{ n.x, n.y, n.z, 0.f };
        Vec4f t = aPreTransform * n4;
        Vec3f nn{ t.x, t.y, t.z };
        n = normalize(nn);
    }

    // ----- Fill SimpleMeshData (same pattern as cylinder/cone) -----
    SimpleMeshData mesh;
    mesh.positions = std::move( position );
    mesh.normals = std::move( normal );
    mesh.colors.resize( mesh.positions.size(), aColor );
    mesh.Ka.resize( mesh.positions.size(), Ka );
    mesh.Kd.resize( mesh.positions.size(), Kd );
    mesh.Ke.resize( mesh.positions.size(), Ke );
    mesh.Ks.resize( mesh.positions.size(), Ks );
    mesh.Ns.resize( mesh.positions.size(), Ns);

    return mesh;
}