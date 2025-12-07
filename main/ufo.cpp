// ufo.cpp  (SKELETON – you MUST edit / complete the TODOs yourself)

#include "ufo.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

namespace
{
    // ----------------------------------------------------------------
    // Helper: push one flat-shaded triangle
    // ----------------------------------------------------------------
    void pushTriangleFlat(
        std::vector<Vec3f>& positions,
        std::vector<Vec3f>& normals,
        Vec3f const& p0,
        Vec3f const& p1,
        Vec3f const& p2,
        Vec3f normal )
    {
    // Make sure the normal is unit length
    normal = normalize(-normal);

    // Push the three vertices
    positions.push_back(p0);
    positions.push_back(p1);
    positions.push_back(p2);

    // Flat shading: same normal for all three
    normals.push_back(normal);
    normals.push_back(normal);
    normals.push_back(normal);
    }

    // ----------------------------------------------------------------
    // Cylinder: y from y0 to y1, radius in XZ, centred on Y axis
    // ----------------------------------------------------------------
    void addCylinder(
        std::vector<Vec3f>& positions,
        std::vector<Vec3f>& normals,
        int slices,
        float radius,
        float y0,
        float y1 )
    {
        float const twoPi = 2.0f * std::numbers::pi_v<float>;

        // Side
        for (int i = 0; i < slices; ++i)
        {
            float a0 = twoPi * float(i) / float(slices);
            float a1 = twoPi * float(i + 1) / float(slices);

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

            pushTriangleFlat(positions, normals, p00, p10, p11, nAvg);
            pushTriangleFlat(positions, normals, p00, p11, p01, nAvg);
        }

        // Bottom cap
        {
            Vec3f center{ 0.0f, y0, 0.0f };
            Vec3f n{ 0.0f, -1.0f, 0.0f };

            for (int i = 0; i < slices; ++i)
            {
                float a0 = twoPi * float(i) / float(slices);
                float a1 = twoPi * float(i + 1) / float(slices);

                Vec3f p0{ radius * std::cos(a0), y0, radius * std::sin(a0) };
                Vec3f p1{ radius * std::cos(a1), y0, radius * std::sin(a1) };

                pushTriangleFlat(positions, normals, center, p0, p1, n);
            }
        }

        // Top cap
        {
            Vec3f center{ 0.0f, y1, 0.0f };
            Vec3f n{ 0.0f, 1.0f, 0.0f };

            for (int i = 0; i < slices; ++i)
            {
                float a0 = twoPi * float(i) / float(slices);
                float a1 = twoPi * float(i + 1) / float(slices);

                Vec3f p0{ radius * std::cos(a0), y1, radius * std::sin(a0) };
                Vec3f p1{ radius * std::cos(a1), y1, radius * std::sin(a1) };

                pushTriangleFlat(positions, normals, center, p1, p0, n);
            }
        }
    }

    // ----------------------------------------------------------------
    // Cone: base at yBase (radius > 0), tip at yTip on Y axis
    // ----------------------------------------------------------------
    void addCone(
        std::vector<Vec3f>& positions,
        std::vector<Vec3f>& normals,
        int slices,
        float baseRadius,
        float yBase,
        float yTip )
    {
        float const twoPi = 2.0f * std::numbers::pi_v<float>;
        Vec3f tip{ 0.0f, yTip, 0.0f };

        // Side
        for (int i = 0; i < slices; ++i)
        {
            float a0 = twoPi * float(i) / float(slices);
            float a1 = twoPi * float(i + 1) / float(slices);

            Vec3f p0{
                baseRadius * std::cos(a0), yBase,
                baseRadius * std::sin(a0)
            };
            Vec3f p1{
                baseRadius * std::cos(a1), yBase,
                baseRadius * std::sin(a1)
            };

            Vec3f e1 = p1 - p0;
            Vec3f e2 = tip - p0;
            Vec3f n  = cross(e2, e1);  // TODO: this uses your Vec3f::cross

            pushTriangleFlat(positions, normals, p0, tip, p1, n);
        }

        // Base disc
        Vec3f center{ 0.0f, yBase, 0.0f };
        Vec3f nb{ 0.0f, -1.0f, 0.0f };

        for (int i = 0; i < slices; ++i)
        {
            float a0 = twoPi * float(i) / float(slices);
            float a1 = twoPi * float(i + 1) / float(slices);

            Vec3f p0{
                baseRadius * std::cos(a0), yBase,
                baseRadius * std::sin(a0)
            };
            Vec3f p1{
                baseRadius * std::cos(a1), yBase,
                baseRadius * std::sin(a1)
            };

            pushTriangleFlat(positions, normals, center, p0, p1, nb);
        }
    }

    // ----------------------------------------------------------------
    // Axis-aligned box: centre c, half extents (hx, hy, hz)
    // ----------------------------------------------------------------
    void addBox(
        std::vector<Vec3f>& positions,
        std::vector<Vec3f>& normals,
        Vec3f c,
        float hx, float hy, float hz )
    {
        // Compute 8 corners
        Vec3f p000{ c.x - hx, c.y - hy, c.z - hz };
        Vec3f p001{ c.x - hx, c.y - hy, c.z + hz };
        Vec3f p010{ c.x - hx, c.y + hy, c.z - hz };
        Vec3f p011{ c.x - hx, c.y + hy, c.z + hz };
        Vec3f p100{ c.x + hx, c.y - hy, c.z - hz };
        Vec3f p101{ c.x + hx, c.y - hy, c.z + hz };
        Vec3f p110{ c.x + hx, c.y + hy, c.z - hz };
        Vec3f p111{ c.x + hx, c.y + hy, c.z + hz };

               // +X face (normal +X, CCW when viewed from +X)
        pushTriangleFlat(positions, normals,
                         p101, p100, p110, Vec3f{ 1.0f, 0.0f, 0.0f });
        pushTriangleFlat(positions, normals,
                         p101, p110, p111, Vec3f{ 1.0f, 0.0f, 0.0f });

        // -X face (normal -X, CCW when viewed from -X)
        pushTriangleFlat(positions, normals,
                         p000, p001, p011, Vec3f{ -1.0f, 0.0f, 0.0f });
        pushTriangleFlat(positions, normals,
                         p000, p011, p010, Vec3f{ -1.0f, 0.0f, 0.0f });

        // +Y face (top, normal +Y)
        pushTriangleFlat(positions, normals,
                         p110, p010, p011, Vec3f{ 0.0f, 1.0f, 0.0f });
        pushTriangleFlat(positions, normals,
                         p110, p011, p111, Vec3f{ 0.0f, 1.0f, 0.0f });

        // -Y face (bottom, normal -Y)
        pushTriangleFlat(positions, normals,
                         p000, p100, p101, Vec3f{ 0.0f, -1.0f, 0.0f });
        pushTriangleFlat(positions, normals,
                         p000, p101, p001, Vec3f{ 0.0f, -1.0f, 0.0f });

        // +Z face (front, normal +Z)
        pushTriangleFlat(positions, normals,
                         p001, p101, p111, Vec3f{ 0.0f, 0.0f, 1.0f });
        pushTriangleFlat(positions, normals,
                         p001, p111, p011, Vec3f{ 0.0f, 0.0f, 1.0f });

        // -Z face (back, normal -Z)
        pushTriangleFlat(positions, normals,
                         p000, p110, p100, Vec3f{ 0.0f, 0.0f, -1.0f });
        pushTriangleFlat(positions, normals,
                         p000, p010, p110, Vec3f{ 0.0f, 0.0f, -1.0f });

    }
} // anonymous namespace


// ===================================================================
// PUBLIC ENTRY POINT
// ===================================================================
void buildUfoFlatArrays(
    std::vector<Vec3f>& outPositions,
    std::vector<Vec3f>& outNormals,
    int& outBaseVertexCount,
    int& outTopVertexCount, 
    int& outBodyStart,
    int& outBodyCount,
    int& outEngineStart,
    int& outEngineCount,
    int& outFinsStart,
    int& outFinsCount,
    int& outBulbsStart,
    int& outBulbsCount,
    int& outTopStart,
    int& outTopCount)
{
    outPositions.clear();
    outNormals.clear();

    // -------- rocket dimensions ----------
    int   const slices       = 600;
    float const bodyRadius   = 0.60f;
    float const bodyHeight   = 4.0f;
    float const engineHeight = 0.5f;
    float const noseHeight   = 2.5f;
    float const finHeight    = 1.0f;
    float const finThickness = 0.05f;
    float const finWidth     = 0.9f;

    float const bodyBottomY  = -bodyHeight * 0.5f;
    float const bodyTopY     =  bodyHeight * 0.5f;

  // ---------------- BASE PARTS ----------------
    std::size_t baseStart = outPositions.size();

    // 1) main body
    std::size_t bodyStart = outPositions.size();
    addCylinder(
        outPositions, outNormals,
        slices,
        bodyRadius,
        bodyBottomY,
        bodyTopY
    );
    std::size_t bodyEnd = outPositions.size();
    // 2) engine at bottom
    std::size_t engineStart = outPositions.size();
    float const engineRadius  = bodyRadius * 0.7f;
    float const engineTopY    = bodyBottomY;
    float const engineBottomY = engineTopY - engineHeight;

    addCylinder(
        outPositions, outNormals,
        slices,
        engineRadius,
        engineBottomY,
        engineTopY
    );
    std::size_t engineEnd = outPositions.size();

    // 3) four fins (boxes)
    std::size_t finsStart = outPositions.size();

    float const finCenterY = bodyBottomY + finHeight * 0.5f;
    float const finOffset  = bodyRadius + finThickness * 0.5f;

    // +X
    addBox(
        outPositions, outNormals,
        Vec3f{ 0.0f, finCenterY, -finOffset },
        finThickness * 0.5f,
        finHeight   * 0.5f,
        finWidth    * 0.5f
    );
    // -X
    addBox(
        outPositions, outNormals,
        Vec3f{ 0.f, finCenterY, finOffset },
        finThickness * 0.5f,
        finHeight   * 0.5f,
        finWidth    * 0.5f
    );
    // +Z
    addBox(
        outPositions, outNormals,
        Vec3f{ finOffset, finCenterY, 0.0f },
        finWidth    * 0.5f,
        finHeight   * 0.5f,
        finThickness * 0.5f
    );
    // -Z
    addBox(
        outPositions, outNormals,
        Vec3f{ -finOffset, finCenterY, 0.0f },
        finWidth    * 0.5f,
        finHeight   * 0.5f,
        finThickness * 0.5f
    );

    std::size_t finsEnd = outPositions.size();

    // 4) three small light bulbs (boxes)
    std::size_t bulbsStart = outPositions.size();

    // Light 1  0 degrees
    addBox(
        outPositions, outNormals,
        Vec3f{ -finOffset, finCenterY+3.f, 0.0f },
        0.1f,
        0.1f,
        0.1f
    );

    // Light 2   2pi/3 = 120 degrees
    addBox(
        outPositions, outNormals,
        Vec3f{ 0.5f * finOffset, finCenterY+3.f,  -0.8660254f * finOffset},
        0.1f,
        0.1f,
        0.1f
    );

    // Light 3   4pi/3 = 240 degrees
    addBox(
        outPositions, outNormals,
        Vec3f{ 0.5f * finOffset, finCenterY+3.f,  0.8660254f * finOffset },
        0.1f,
        0.1f,
        0.1f
    );

    std::size_t bulbsEnd = outPositions.size();

    // Mark base-vertex count
    outBaseVertexCount = static_cast<int>(outPositions.size() - baseStart);
    // ------------- TOP PARTS -------------
       std::size_t topStart = outPositions.size();
   
       // 4) nose cone, sitting on top of body
       float const noseBaseY = bodyTopY;
       float const noseTipY  = noseBaseY + noseHeight;
   
       addCone(
           outPositions, outNormals,
           slices,
           bodyRadius * 1.0f,
           noseBaseY,
           noseTipY
       );
   
       // 5) antenna cylinder above nose
       float const antennaRadius = 0.05f;
       float const antennaHeight = 0.2f;
       float const antennaBaseY  = noseTipY;
       float const antennaTopY   = antennaBaseY + antennaHeight;
   
       addCylinder(
           outPositions, outNormals,
           slices,
           antennaRadius,
           antennaBaseY-0.3f,
           antennaTopY
       );
   
       // 6) tiny cone tip at very top
       float const tipRadius = 0.05f;
       float const tipBaseY  = antennaTopY;
       float const tipTopY   = tipBaseY + 0.5f;
   
       addCone(
           outPositions, outNormals,
           slices,
           tipRadius,
           tipBaseY,
           tipTopY
       );
   
       std::size_t topEnd = outPositions.size();
   
       outTopVertexCount = static_cast<int>(topEnd - topStart); 
       
       // Export all the ranges as ints for main.cpp
           outBodyStart   = static_cast<int>(bodyStart);
           outBodyCount   = static_cast<int>(bodyEnd - bodyStart);
       
           outEngineStart = static_cast<int>(engineStart);
           outEngineCount = static_cast<int>(engineEnd - engineStart);
       
           outFinsStart   = static_cast<int>(finsStart);
           outFinsCount   = static_cast<int>(finsEnd - finsStart);
       
           outBulbsStart  = static_cast<int>(bulbsStart);
           outBulbsCount  = static_cast<int>(bulbsEnd - bulbsStart);
       
           outTopStart    = static_cast<int>(topStart);
           outTopCount    = static_cast<int>(topEnd - topStart);

}