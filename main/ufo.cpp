#include "ufo.hpp"

#include <numbers>   // for std::numbers::pi_v

// ---------- small math helpers ----------------------------------------

static Vec3f transform_point(Mat44f const& M, Vec3f const& p)
{
    Vec4f p4{ p.x, p.y, p.z, 1.f };
    Vec4f r4 = M * p4;
    return Vec3f{ r4.x, r4.y, r4.z };
}

static Vec3f cross(Vec3f a, Vec3f b) noexcept
{
    return Vec3f{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static Mat44f make_transform(Vec3f translation,
                             Vec3f rotationDegrees,
                             Vec3f scale)
{
    float const pi = std::numbers::pi_v<float>;
    float const deg2rad = pi / 180.f;

    Mat44f S = make_scaling(scale.x, scale.y, scale.z);

    Mat44f Rx = make_rotation_x(rotationDegrees.x * deg2rad);
    Mat44f Ry = make_rotation_y(rotationDegrees.y * deg2rad);
    Mat44f Rz = make_rotation_z(rotationDegrees.z * deg2rad);

    Mat44f R = Rz * Ry * Rx;   // Z then Y then X
    Mat44f T = make_translation(translation);

    // Model = T * R * S
    return T * R * S;
}

// ---------- normal recomputation --------------------------------------

static void recompute_normals(std::vector<Vertex>& vertices,
                              std::vector<Index>&  indices)
{
    // zero
    for (auto& v : vertices)
        v.normal = Vec3f{ 0.f, 0.f, 0.f };

    // accumulate per-face normals
    for (std::size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        Index i0 = indices[i + 0];
        Index i1 = indices[i + 1];
        Index i2 = indices[i + 2];

        Vec3f const& p0 = vertices[i0].position;
        Vec3f const& p1 = vertices[i1].position;
        Vec3f const& p2 = vertices[i2].position;

        Vec3f e1 = p1 - p0;
        Vec3f e2 = p2 - p0;
        Vec3f n  = normalize( cross(e1, e2) );

        vertices[i0].normal += n;
        vertices[i1].normal += n;
        vertices[i2].normal += n;
    }

    // normalize
    for (auto& v : vertices)
        v.normal = normalize(v.normal);
}

// ---------- basic shape: disc (saucer-like cylinder) -------------------

static void addDisc(std::vector<Vertex>& vertices,
                    std::vector<Index>&  indices,
                    Mat44f const&        M,
                    Vec3f const&         baseColor,
                    int                  slices)
{
    Index baseIndex = static_cast<Index>(vertices.size());

    float const radius     = 0.5f;
    float const halfHeight = 0.5f;

    for (int i = 0; i <= slices; ++i)
    {
        float angle = (2.f * std::numbers::pi_v<float> * i) / float(slices);
        float c = std::cos(angle);
        float s = std::sin(angle);

        Vec3f localTop   { radius * c,  halfHeight, radius * s };
        Vec3f localBottom{ radius * c, -halfHeight, radius * s };

        Vec3f worldTop    = transform_point(M, localTop);
        Vec3f worldBottom = transform_point(M, localBottom);

        Vec3f nLocal{ c, 0.f, s };  // radial in XZ

        Vertex vTop;
        vTop.position = worldTop;
        vTop.normal   = nLocal;
        vTop.color    = baseColor;

        Vertex vBottom;
        vBottom.position = worldBottom;
        vBottom.normal   = nLocal;
        vBottom.color    = baseColor;

        vertices.push_back(vTop);
        vertices.push_back(vBottom);
    }

    for (int i = 0; i < slices; ++i)
    {
        Index i0 = baseIndex + 2*i;
        Index i1 = baseIndex + 2*i + 1;
        Index i2 = baseIndex + 2*(i+1);
        Index i3 = baseIndex + 2*(i+1) + 1;

        // tri 1
        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);

        // tri 2
        indices.push_back(i2);
        indices.push_back(i1);
        indices.push_back(i3);
    }
}

// ---------- UFO builder ------------------------------------------------

void buildUfo(std::vector<Vertex>& outVertices,
              std::vector<Index>&  outIndices)
{
    outVertices.clear();
    outIndices.clear();

    // 1) Main saucer
    {
        Vec3f pos   { 0.f, 0.f, 0.f };
        Vec3f rot   { 0.f, 0.f, 0.f };
        Vec3f scale { 4.f, 0.5f, 4.f };   // wide & flat

        Mat44f M = make_transform(pos, rot, scale);

        addDisc(outVertices, outIndices, M,
                Vec3f{ 0.7f, 0.7f, 0.8f },
                48);
    }

    // 2) Top dome
    {
        Vec3f pos   { 0.f, 0.7f, 0.f };
        Vec3f rot   { 0.f, 0.f, 0.f };
        Vec3f scale { 1.5f, 0.8f, 1.5f };

        Mat44f M = make_transform(pos, rot, scale);

        addDisc(outVertices, outIndices, M,
                Vec3f{ 0.3f, 0.6f, 0.9f },
                32);
    }

    // 3) Bottom bulge
    {
        Vec3f pos   { 0.f, -0.5f, 0.f };
        Vec3f rot   { 0.f, 0.f, 0.f };
        Vec3f scale { 2.5f, 0.4f, 2.5f };

        Mat44f M = make_transform(pos, rot, scale);

        addDisc(outVertices, outIndices, M,
                Vec3f{ 0.6f, 0.6f, 0.7f },
                32);
    }

    // 4) Light ring
    {
        int   lightCount = 8;
        float radius     = 3.8f;
        float y          = -0.1f;

        for (int i = 0; i < lightCount; ++i)
        {
            float angle = (2.f * std::numbers::pi_v<float> * i) / float(lightCount);
            float c = std::cos(angle);
            float s = std::sin(angle);

            Vec3f pos   { radius * c, y, radius * s };
            Vec3f rot   { 0.f, -angle * 180.f / std::numbers::pi_v<float>, 0.f };
            Vec3f scale { 0.3f, 0.1f, 0.3f };

            Mat44f M = make_transform(pos, rot, scale);

            addDisc(outVertices, outIndices, M,
                    Vec3f{ 1.f, 1.f, 0.3f },   // lights
                    12);
        }
    }

    // TODO later: legs using addBox(...) if you implement boxes

    // Make normals consistent for lighting
    recompute_normals(outVertices, outIndices);
}
