#include "ufo.hpp"

#include <numbers>

// Internal vertex/index representation for building the UFO
struct UfoVertex
{
    Vec3f position;
    Vec3f normal;
};

using UfoIndex = std::uint32_t;

// ========== small helpers ======================================

static Vec3f transform_point(Mat44f const& M, Vec3f const& p)
{
    Vec4f p4{ p.x, p.y, p.z, 1.f };
    Vec4f r = M * p4;
    return Vec3f{ r.x, r.y, r.z };
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

    Mat44f R = Rz * Ry * Rx;
    Mat44f T = make_translation(translation);

    return T * R * S; // model = T * R * S
}

static Vec3f cross_(Vec3f a, Vec3f b) noexcept
{
    return Vec3f{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static void recompute_normals(std::vector<UfoVertex>& vertices,
                              std::vector<UfoIndex>&  indices)
{
    for (auto& v : vertices)
        v.normal = Vec3f{ 0.f, 0.f, 0.f };

    for (std::size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        UfoIndex i0 = indices[i + 0];
        UfoIndex i1 = indices[i + 1];
        UfoIndex i2 = indices[i + 2];

        Vec3f const& p0 = vertices[i0].position;
        Vec3f const& p1 = vertices[i1].position;
        Vec3f const& p2 = vertices[i2].position;

        Vec3f e1 = p1 - p0;
        Vec3f e2 = p2 - p0;
        Vec3f n  = normalize( cross_(e1, e2) );

        vertices[i0].normal += n;
        vertices[i1].normal += n;
        vertices[i2].normal += n;
    }

    for (auto& v : vertices)
        v.normal = normalize(v.normal);
}

// ========== primitive: disc with caps ===========================

// Adds a short cylinder with top/bottom caps (like a fat disc)
static void addDisc(std::vector<UfoVertex>& vertices,
                    std::vector<UfoIndex>&  indices,
                    Mat44f const&           M,
                    float                   radius,
                    float                   height,
                    int                     slices)
{
    UfoIndex baseIndex = static_cast<UfoIndex>(vertices.size());

    float const halfH = height * 0.5f;

    // center top and bottom (for caps)
    Vec3f topCenterLocal{ 0.f,  halfH, 0.f };
    Vec3f botCenterLocal{ 0.f, -halfH, 0.f };

    Vec3f topCenterWorld = transform_point(M, topCenterLocal);
    Vec3f botCenterWorld = transform_point(M, botCenterLocal);

    UfoVertex vTopC{ topCenterWorld, Vec3f{0.f, 1.f, 0.f} };
    UfoVertex vBotC{ botCenterWorld, Vec3f{0.f,-1.f, 0.f} };

    UfoIndex idxTopCenter = baseIndex;
    UfoIndex idxBotCenter = baseIndex + 1;

    vertices.push_back(vTopC);
    vertices.push_back(vBotC);

    // ring vertices
    for (int i = 0; i <= slices; ++i)
    {
        float angle = (2.f * std::numbers::pi_v<float> * i) / float(slices);
        float c = std::cos(angle);
        float s = std::sin(angle);

        Vec3f localTop   { radius * c,  halfH, radius * s };
        Vec3f localBottom{ radius * c, -halfH, radius * s };

        Vec3f worldTop    = transform_point(M, localTop);
        Vec3f worldBottom = transform_point(M, localBottom);

        // approximate radial normal
        Vec3f nSide{ c, 0.f, s };

        UfoVertex vTop;
        vTop.position = worldTop;
        vTop.normal   = nSide;

        UfoVertex vBot;
        vBot.position = worldBottom;
        vBot.normal   = nSide;

        vertices.push_back(vTop);
        vertices.push_back(vBot);
    }

    // indices: caps + side
    // top cap fan
    for (int i = 0; i < slices; ++i)
    {
        UfoIndex i1 = baseIndex + 2 + 2*i;     // top ring i
        UfoIndex i2 = baseIndex + 2 + 2*(i+1); // top ring i+1

        indices.push_back(idxTopCenter);
        indices.push_back(i1);
        indices.push_back(i2);
    }

    // bottom cap fan
    for (int i = 0; i < slices; ++i)
    {
        UfoIndex i1 = baseIndex + 3 + 2*i;     // bottom ring i
        UfoIndex i2 = baseIndex + 3 + 2*(i+1); // bottom ring i+1

        indices.push_back(idxBotCenter);
        indices.push_back(i2);
        indices.push_back(i1);
    }

    // side quads
    for (int i = 0; i < slices; ++i)
    {
        UfoIndex top0 = baseIndex + 2 + 2*i;
        UfoIndex bot0 = baseIndex + 3 + 2*i;
        UfoIndex top1 = baseIndex + 2 + 2*(i+1);
        UfoIndex bot1 = baseIndex + 3 + 2*(i+1);

        // tri 1
        indices.push_back(top0);
        indices.push_back(bot0);
        indices.push_back(top1);

        // tri 2
        indices.push_back(top1);
        indices.push_back(bot0);
        indices.push_back(bot1);
    }
}

// ========== build UFO, then flatten to triangle list ===========

void buildUfoFlatArrays(std::vector<Vec3f>& outPositions,
                        std::vector<Vec3f>& outNormals)
{
    std::vector<UfoVertex> verts;
    std::vector<UfoIndex>  idx;

    verts.reserve(2000);
    idx.reserve(6000);

    // 1) Main saucer
    {
        Vec3f pos   { 0.f, 0.f, 0.f };
        Vec3f rot   { 0.f, 0.f, 0.f };
        Vec3f scale { 6.f, 1.0f, 6.f };  // adjust to taste

        Mat44f M = make_transform(pos, rot, scale);

        addDisc(verts, idx, M, 1.0f, 0.4f, 48);
    }

    // 2) Top dome
    {
        Vec3f pos   { 0.f, 0.7f, 0.f };
        Vec3f rot   { 0.f, 0.f, 0.f };
        Vec3f scale { 2.0f, 1.0f, 2.0f };

        Mat44f M = make_transform(pos, rot, scale);

        addDisc(verts, idx, M, 0.6f, 0.3f, 32);
    }

    // 3) Bottom bulge
    {
        Vec3f pos   { 0.f, -0.5f, 0.f };
        Vec3f rot   { 0.f, 0.f, 0.f };
        Vec3f scale { 3.0f, 0.8f, 3.0f };

        Mat44f M = make_transform(pos, rot, scale);

        addDisc(verts, idx, M, 0.7f, 0.3f, 32);
    }

    // Compute normals
    recompute_normals(verts, idx);

    // Flatten into simple triangle list: positions + normals
    outPositions.clear();
    outNormals.clear();
    outPositions.reserve(idx.size());
    outNormals.reserve(idx.size());

    for (auto i : idx)
    {
        outPositions.push_back(verts[i].position);
        outNormals.push_back(verts[i].normal);
    }
}
