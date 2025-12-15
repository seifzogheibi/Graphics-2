#include "spaceship.hpp"
#include "shapes.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

// creating a cylinder with x = 0,1 length axis and y,z radius 1
SimpleMeshData make_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)
{
    // position of each vertex of a triangle
    std::vector<Vec3f> position;
    // normals for lighting (same as position amount)
    std::vector<Vec3f> normal;

    // start at angle 0 (y=1, z=0)
    float prevY = std::cos( 0.f );
    float prevZ = std::sin( 0.f );
    
    // create subdivisions to build the cylinder shell (two triangles create one subdiv/segment of the cylinder)
    for( std::size_t i = 0; i < aSubdivs; ++i )
    {
    // compute angle of next subdivision (up to 2pi)
     float const angle = (i+1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
    //  convert angle to y,z coordinates on unit circle
     float y = std::cos( angle );
     float z = std::sin( angle );
     
    // average normal for the two triangles of a segment (x is 0 for a cylinder along x axis)
     Vec3f nAvg{
        0.f,
        0.5f * (prevY + y),
        0.5f * (prevZ + z)
    };
    // normalize for lighting
    nAvg = normalize( nAvg );

    // Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
    position.emplace_back(Vec3f{ 0.f, prevY, prevZ });
    position.emplace_back(Vec3f{ 0.f, y, z });
	position.emplace_back(Vec3f{ 1.f, prevY, prevZ });
    // all vertices hvae the same normal
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );

    // second triangle of the segment
	position.emplace_back(Vec3f{ 0.f, y, z });
	position.emplace_back(Vec3f{ 1.f, y, z });
	position.emplace_back(Vec3f{ 1.f, prevY, prevZ });
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );
    normal.emplace_back( nAvg );

    // move to next segment
    prevY = y;
    prevZ = z;
    }

    if (aCapped)
    {
        // reset to first point 
        prevY = std::cos(0.f);
        prevZ = std::sin(0.f);

        // bottom center of cylinder (to cap the top, same points but x=1 and positive normal)
        Vec3f bottom{ 0.f, 0.f, 0.f };
        // normal pointing down x axis
        Vec3f bottom_n{ -1.f, 0.f, 0.f };
        

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);
            // start point
            Vec3f p0{ 0.f, prevY, prevZ };
            // next point
            Vec3f p1{ 0.f, y,z};
        
            position.emplace_back(bottom);
            position.emplace_back(p1);
            position.emplace_back(p0);

            normal.emplace_back(bottom_n);
            normal.emplace_back(bottom_n);
            normal.emplace_back(bottom_n);

            prevY = y;
            prevZ = z;
        }
    }

    // pre-transform positions
    for( auto& p : position )
    {
    // convert to Vec4f for matrix multiplication
    Vec4f p4{ p.x, p.y, p.z, 1.f };
    // apply pre-transform
    Vec4f t = aPreTransform * p4;
    t /= t.w;
    // convert back to Vec3f
    p = Vec3f{ t.x, t.y, t.z };
    }
    // pre-transform normals
    for( auto& n : normal )
    {
    Vec4f n4{ n.x, n.y, n.z, 0.f };
    Vec4f t = aPreTransform * n4;
    n = Vec3f{ t.x, t.y, t.z };
    n = normalize( n );
    }

    // place vectors into mesh
    SimpleMeshData mesh;
    // move to avoid copies
    mesh.positions = std::move( position );
    mesh.normals = std::move( normal );
    // apply material properties
    mesh.colors.resize( mesh.positions.size(), aColor );
    mesh.Ka.resize( mesh.positions.size(), Ka );
    mesh.Kd.resize( mesh.positions.size(), Kd );
    mesh.Ke.resize( mesh.positions.size(), Ke );
    mesh.Ks.resize( mesh.positions.size(), Ks );
    mesh.Ns.resize( mesh.positions.size(), Ns );

    return mesh;
}

SimpleMeshData make_cone(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)
{
    std::vector<Vec3f> position;
    std::vector<Vec3f> normal;

    float prevY = std::cos( 0.f );
    float prevZ = std::sin( 0.f );    

    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos(angle);
        float z = std::sin(angle);

        Vec3f tip{ 1.f, 0.f, 0.f };
        Vec3f p0{ 0.f, prevY, prevZ };
        Vec3f p1{ 0.f, y, z };

        position.emplace_back(p0);
        position.emplace_back(p1);
        position.emplace_back(tip);

             Vec3f nAvg{
        0.f,
        0.5f * (prevY + y),
        0.5f * (prevZ + z)
    };
    nAvg = normalize( nAvg );


        normal.emplace_back(nAvg);
        normal.emplace_back(nAvg);
        normal.emplace_back(nAvg);

        prevY = y;
        prevZ = z;
    }


    if (aCapped)
    {
        prevY = std::cos(0.f);
        prevZ = std::sin(0.f);

        Vec3f bottom{ 0.f, 0.f, 0.f };
        Vec3f nAvg{ -1.f, 0.f, 0.f };

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);

            Vec3f p0{ 0.f, prevY, prevZ };
            Vec3f p1{ 0.f, y, z };

            position.emplace_back(bottom);
            position.emplace_back(p1);
            position.emplace_back(p0);
            normal.emplace_back(nAvg);
            normal.emplace_back(nAvg);
            normal.emplace_back(nAvg);

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

    // fin is flat in one dimension so we can define half thickness
    const float halfT = 0.5f;

    // front side (z = +halfT)
    Vec3f p0f{ 0.f, 0.f,  halfT };
    Vec3f p2f{ 1.f, 0.f,  halfT };
    Vec3f p1f{ 0.f, 1.f,  halfT };

    // back side (z = -halfT)
    Vec3f p0b{ 0.f, 0.f, -halfT };
    Vec3f p2b{ 1.f, 0.f, -halfT };
    Vec3f p1b{ 0.f, 1.f, -halfT };

    {
        // edges of the front face
        Vec3f e0 = p2f - p0f;
        Vec3f e1 = p1f - p0f;
        // normal for fornt is cross product of two edges
        Vec3f nFront = normalize(cross(e0, e1));

        // draw front side
        position.emplace_back(p0f);
        position.emplace_back(p2f);
        position.emplace_back(p1f);
        normal.emplace_back(nFront);
        normal.emplace_back(nFront);
        normal.emplace_back(nFront);
    }

    {
        // edges of the back face
        Vec3f e0 = p1b - p0b;
        Vec3f e1 = p2b - p0b;
        Vec3f nBack = normalize(cross(e0, e1));

        // draw back side
        position.emplace_back(p0b);
        position.emplace_back(p1b);
        position.emplace_back(p2b);
        normal.emplace_back(nBack);
        normal.emplace_back(nBack);
        normal.emplace_back(nBack);
    }

    {
        // bottom face (connecting front and back)
        Vec3f e0 = p2b - p0b;
        Vec3f e1 = p2f - p0b;
        Vec3f nBase = normalize(cross(e0, e1));

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

    {
        // side face
        Vec3f e0 = p1f - p0b;
        Vec3f e1 = p1b - p0b;
        Vec3f nSide = normalize(cross(e0, e1));

        position.emplace_back(p0b);
        position.emplace_back(p1f);
        position.emplace_back(p1b);
        normal.emplace_back(nSide);
        normal.emplace_back(nSide);
        normal.emplace_back(nSide);

        position.emplace_back(p0b);
        position.emplace_back(p0f);
        position.emplace_back(p1f);
        normal.emplace_back(nSide);
        normal.emplace_back(nSide);
        normal.emplace_back(nSide);
    }

    {
        // hypotenuse face
        Vec3f e0 = p1b - p2b;
        Vec3f e1 = p1f - p2b;
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


// making a unit cube centred at origin of size 1 (width/height/length)
SimpleMeshData make_cube(bool aCapped, std::size_t aSubdivs, Vec3f aColor,Mat44f aPreTransform, float Ns, Vec3f Ka, Vec3f Kd, Vec3f Ke, Vec3f Ks)
{
    std::vector<Vec3f> position;
    std::vector<Vec3f> normal;

    // define 8 vertices of the cube
    // since lenght/width/height = 1 around the origin, vertices are at +/-0.5 in each dimension
    Vec3f v000{ -0.5f, -0.5f, -0.5f };
    Vec3f v001{ -0.5f, -0.5f,  0.5f };
    Vec3f v010{ -0.5f,  0.5f, -0.5f };
    Vec3f v011{ -0.5f,  0.5f,  0.5f };
    Vec3f v100{  0.5f, -0.5f, -0.5f };
    Vec3f v101{  0.5f, -0.5f,  0.5f };
    Vec3f v110{  0.5f,  0.5f, -0.5f };
    Vec3f v111{  0.5f,  0.5f,  0.5f };

    // helper to add a face (two triangles) given 3 vertices and a normal
    auto add_face = [&](const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& n)
    {
        // create one triangle 
        position.emplace_back(a); 
        position.emplace_back(b); 
        position.emplace_back(c); 
        normal.emplace_back(n);
        normal.emplace_back(n);
        normal.emplace_back(n);
    };

    // each side uses the helper to add two triangles per face
    // right side face
    add_face(v100, v111, v101, Vec3f{ 1.f, 0.f, 0.f });
    add_face(v100, v110, v111, Vec3f{ 1.f, 0.f, 0.f });

    // left side face
    add_face(v000, v011, v010, Vec3f{ -1.f, 0.f, 0.f });
    add_face(v000, v001, v011, Vec3f{ -1.f, 0.f, 0.f });

    // top face
    add_face(v010, v111, v110, Vec3f{ 0.f, 1.f, 0.f });
    add_face(v010, v011, v111, Vec3f{ 0.f, 1.f, 0.f });

    // bottom face
    add_face(v000, v101, v001, Vec3f{ 0.f,-1.f, 0.f });
    add_face(v000, v100, v101, Vec3f{ 0.f,-1.f, 0.f });

    // front face
    add_face(v001, v111, v011, Vec3f{ 0.f, 0.f, 1.f });
    add_face(v001, v101, v111, Vec3f{ 0.f, 0.f, 1.f });

    // back
    add_face(v000, v110, v100, Vec3f{ 0.f, 0.f,-1.f });
    add_face(v000, v010, v110, Vec3f{ 0.f, 0.f,-1.f });


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
