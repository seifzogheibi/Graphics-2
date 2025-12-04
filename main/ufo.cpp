#include "ufo.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"

// We build a saucer with:
//  - bulged side band
//  - domed top
//  - domed bottom
// All as non-indexed triangles: 3 vertices per triangle in outPositions/outNormals.

namespace
{
    // Push a single triangle with a constant normal
    void pushTriangleFlat(std::vector<Vec3f>& positions,
                          std::vector<Vec3f>& normals,
                          Vec3f const& p0,
                          Vec3f const& p1,
                          Vec3f const& p2,
                          Vec3f n)
    {
        // make sure normal is unit length
        n = normalize(n);

        positions.push_back(p0);
        positions.push_back(p1);
        positions.push_back(p2);

        normals.push_back(n);
        normals.push_back(n);
        normals.push_back(n);
    }
}

void buildUfoFlatArrays(std::vector<Vec3f>& outPositions,
                        std::vector<Vec3f>& outNormals,
                        int& outBaseVertexCount,
                        int& outTopVertexCount)
{
    outPositions.clear();
    outNormals.clear();
    
    int   const segments    = 48;
    // --- Shape parameters you can tweak ---
    float const radiusBody       = 2.0f;   // base radius of the saucer body
    float const bulgeExtra       = 0.7f;   // extra radius at mid-height
    float const halfBodyHeight   = 0.25f;  // half thickness of the mid-band

    float const topCapHeight     = 0.8f;   // height of top dome
    float const bottomCapHeight  = 0.6f;   // height of bottom dome (often smaller)

    int   const segmentsTheta    = 64;     // around Y
    int   const segmentsSideY    = 4;      // vertical slices in side band
    int   const segmentsCapY     = 6;      // vertical slices in each dome

    float const twoPi = 2.0f * std::numbers::pi_v<float>;

    // radius of side band as a function of vertical parameter t in [-1,1]
    auto radiusSideAtT = [&](float t) {
        // t = -1 -> bottom of band  , t = 0 -> middle , t = 1 -> top of band
        // make it bulge in the middle using (1 - t^2)
        float base  = radiusBody;
        float bulge = bulgeExtra * (1.0f - t * t);
        return base + bulge;
    };

    // ----------------------------------------------------
    // 1) Bulged SIDE BAND (between y = -halfBodyHeight and +halfBodyHeight)
    // ----------------------------------------------------
    for (int j = 0; j < segmentsSideY; ++j)
    {
        float v0 = float(j)     / float(segmentsSideY);
        float v1 = float(j + 1) / float(segmentsSideY);

        // y coordinates
        float y0 = -halfBodyHeight + 2.0f * halfBodyHeight * v0;
        float y1 = -halfBodyHeight + 2.0f * halfBodyHeight * v1;

        // map to t in [-1,1]
        float t0 = -1.0f + 2.0f * v0;
        float t1 = -1.0f + 2.0f * v1;

        float r0 = radiusSideAtT(t0);
        float r1 = radiusSideAtT(t1);

        // approximate vertical slope for normals
        float dy   = y1 - y0;
        float dr   = (r1 - r0);
        float slope = (std::fabs(dr) > 1e-4f) ? (dy / dr) : 0.0f;

        for (int i = 0; i < segmentsTheta; ++i)
        {
            float a0 = twoPi * (float(i)     / float(segmentsTheta));
            float a1 = twoPi * (float(i + 1) / float(segmentsTheta));

            float c0 = std::cos(a0);
            float s0 = std::sin(a0);
            float c1 = std::cos(a1);
            float s1 = std::sin(a1);

            Vec3f p00{ r0 * c0, y0, r0 * s0 }; // bottom-left
            Vec3f p01{ r0 * c1, y0, r0 * s1 }; // bottom-right
            Vec3f p10{ r1 * c0, y1, r1 * s0 }; // top-left
            Vec3f p11{ r1 * c1, y1, r1 * s1 }; // top-right

            // radial + small vertical component for smooth-ish shading
            Vec3f n0{ c0, slope, s0 };
            Vec3f n1{ c1, slope, s1 };
            Vec3f nAvg{
                0.5f * (n0.x + n1.x),
                0.5f * (n0.y + n1.y),
                0.5f * (n0.z + n1.z)
            };

            // two triangles forming a quad
            pushTriangleFlat(outPositions, outNormals, p00, p10, p11, nAvg);
            pushTriangleFlat(outPositions, outNormals, p00, p11, p01, nAvg);
        }
    }

    // ----------------------------------------------------
    // 2) TOP DOME (from y = +halfBodyHeight upwards)
    // ----------------------------------------------------
    {
        float const baseY   = +halfBodyHeight;
        float const tipY    = baseY + topCapHeight;

        // precompute rings (y,r)
        std::vector<float> ringY(segmentsCapY + 1);
        std::vector<float> ringR(segmentsCapY + 1);

        for (int j = 0; j <= segmentsCapY; ++j)
        {
            float v = float(j) / float(segmentsCapY); // 0..1
            ringY[j] = baseY + v * topCapHeight;

            // radius shrinks towards tip with a smooth curve
            float k  = v;
            float shrink = 1.0f - 0.85f * k * k; // 1 at base, ~0.15 at tip
            ringR[j] = radiusBody * shrink;
        }

        // rings (except the last) -> quads
        for (int j = 0; j < segmentsCapY; ++j)
        {
            float y0 = ringY[j];
            float y1 = ringY[j + 1];
            float r0 = ringR[j];
            float r1 = ringR[j + 1];

            float dy = y1 - y0;
            float dr = (r1 - r0);
            float slope = (std::fabs(dr) > 1e-4f) ? (dy / dr) : 0.0f;

            for (int i = 0; i < segmentsTheta; ++i)
            {
                float a0 = twoPi * (float(i)     / float(segmentsTheta));
                float a1 = twoPi * (float(i + 1) / float(segmentsTheta));

                float c0 = std::cos(a0);
                float s0 = std::sin(a0);
                float c1 = std::cos(a1);
                float s1 = std::sin(a1);

                Vec3f p00{ r0 * c0, y0, r0 * s0 };
                Vec3f p01{ r0 * c1, y0, r0 * s1 };
                Vec3f p10{ r1 * c0, y1, r1 * s0 };
                Vec3f p11{ r1 * c1, y1, r1 * s1 };

                Vec3f n0{ c0, slope, s0 };
                Vec3f n1{ c1, slope, s1 };
                Vec3f nAvg{
                    0.5f * (n0.x + n1.x),
                    0.5f * (n0.y + n1.y),
                    0.5f * (n0.z + n1.z)
                };

                pushTriangleFlat(outPositions, outNormals, p00, p10, p11, nAvg);
                pushTriangleFlat(outPositions, outNormals, p00, p11, p01, nAvg);
            }
        }

        // Final “tip” – small cap to a point (optional but looks nice)
        Vec3f tip{ 0.0f, tipY, 0.0f };
        float  yRing = ringY.back();
        float  rRing = ringR.back();

        for (int i = 0; i < segmentsTheta; ++i)
        {
            float a0 = twoPi * (float(i)     / float(segmentsTheta));
            float a1 = twoPi * (float(i + 1) / float(segmentsTheta));

            float c0 = std::cos(a0);
            float s0 = std::sin(a0);
            float c1 = std::cos(a1);
            float s1 = std::sin(a1);

            Vec3f p0{ rRing * c0, yRing, rRing * s0 };
            Vec3f p1{ rRing * c1, yRing, rRing * s1 };

            // normal roughly halfway between up and radial
            Vec3f n{
                0.5f * (c0 + c1),
                1.0f,
                0.5f * (s0 + s1)
            };

            pushTriangleFlat(outPositions, outNormals, p0, p1, tip, n);
        }
    }

    // ----------------------------------------------------
    // 3) BOTTOM DOME (mirror of top, usually a bit smaller)
    // ----------------------------------------------------
    {
        float const baseY   = -halfBodyHeight;
        float const tipY    = baseY - bottomCapHeight;

        std::vector<float> ringY(segmentsCapY + 1);
        std::vector<float> ringR(segmentsCapY + 1);

        for (int j = 0; j <= segmentsCapY; ++j)
        {
            float v = float(j) / float(segmentsCapY); // 0..1 from base downwards
            ringY[j] = baseY - v * bottomCapHeight;

            float k  = v;
            float shrink = 1.0f - 0.9f * k * k;
            ringR[j] = radiusBody * shrink;
        }

        for (int j = 0; j < segmentsCapY; ++j)
        {
            float y0 = ringY[j];
            float y1 = ringY[j + 1];
            float r0 = ringR[j];
            float r1 = ringR[j + 1];

            float dy = y1 - y0;
            float dr = (r1 - r0);
            float slope = (std::fabs(dr) > 1e-4f) ? (dy / dr) : 0.0f;

            for (int i = 0; i < segmentsTheta; ++i)
            {
                float a0 = twoPi * (float(i)     / float(segmentsTheta));
                float a1 = twoPi * (float(i + 1) / float(segmentsTheta));

                float c0 = std::cos(a0);
                float s0 = std::sin(a0);
                float c1 = std::cos(a1);
                float s1 = std::sin(a1);

                Vec3f p00{ r0 * c0, y0, r0 * s0 };
                Vec3f p01{ r0 * c1, y0, r0 * s1 };
                Vec3f p10{ r1 * c0, y1, r1 * s0 };
                Vec3f p11{ r1 * c1, y1, r1 * s1 };

                Vec3f n0{ c0, slope, s0 };
                Vec3f n1{ c1, slope, s1 };
                Vec3f nAvg{
                    0.5f * (n0.x + n1.x),
                    0.5f * (n0.y + n1.y),
                    0.5f * (n0.z + n1.z)
                };

                pushTriangleFlat(outPositions, outNormals, p10, p00, p01, nAvg);
                pushTriangleFlat(outPositions, outNormals, p10, p01, p11, nAvg);
            }
        }

        // Tip fan
        Vec3f tip{ 0.0f, tipY, 0.0f };
        float  yRing = ringY.back();
        float  rRing = ringR.back();

        for (int i = 0; i < segmentsTheta; ++i)
        {
            float a0 = twoPi * (float(i)     / float(segmentsTheta));
            float a1 = twoPi * (float(i + 1) / float(segmentsTheta));

            float c0 = std::cos(a0);
            float s0 = std::sin(a0);
            float c1 = std::cos(a1);
            float s1 = std::sin(a1);

            Vec3f p0{ rRing * c0, yRing, rRing * s0 };
            Vec3f p1{ rRing * c1, yRing, rRing * s1 };

            Vec3f n{
                0.5f * (c0 + c1),
               -1.0f,
                0.5f * (s0 + s1)
            };

            pushTriangleFlat(outPositions, outNormals, p1, p0, tip, n);
        }
    }
    // === mark how many vertices belong to the base saucer ===
    outBaseVertexCount = static_cast<int>(outPositions.size());

    // ========================================================
    //  EXTRA: top cap (closing the hole) + antenna
    //  These vertices will be drawn with a different colour.
    // ========================================================

    // --- Top cap (small disc on top) ---
    float const capRadius = 0.7f;               // tweak if you want smaller/larger
    float const capY = halfBodyHeight + topCapHeight + 0.02f;  // slightly above existing top
    Vec3f const capCenter{ 0.0f, capY, 0.0f };
    Vec3f const capNormal{ 0.0f, 1.0f, 0.0f };

    for( int i = 0; i < segments; ++i )
    {
        float a0 = twoPi * (float(i)     / float(segments));
        float a1 = twoPi * (float(i + 1) / float(segments));

        Vec3f p0{
            capRadius * std::cos(a0),
            capY,
            capRadius * std::sin(a0)
        };
        Vec3f p1{
            capRadius * std::cos(a1),
            capY,
            capRadius * std::sin(a1)
        };

        // Triangle fan: center -> p1 -> p0 (CCW from above)
        pushTriangleFlat(outPositions, outNormals,
                         capCenter, p1, p0,
                         capNormal);
    }

    // --- Antenna (thin vertical cylinder) ---
    float const antRadius   = 0.12f;              // thickness of antenna
    float const antHeight   = 1.3f;               // how tall
    float const antBottomY  = capY;
    float const antTopY     = capY + antHeight;
    int   const antSegments = 16;

    for( int i = 0; i < antSegments; ++i )
    {
        float a0 = twoPi * (float(i)     / float(antSegments));
        float a1 = twoPi * (float(i + 1) / float(antSegments));

        float c0 = std::cos(a0);
        float s0 = std::sin(a0);
        float c1 = std::cos(a1);
        float s1 = std::sin(a1);

        Vec3f p0_bot{ antRadius * c0, antBottomY, antRadius * s0 };
        Vec3f p0_top{ antRadius * c0, antTopY,    antRadius * s0 };
        Vec3f p1_bot{ antRadius * c1, antBottomY, antRadius * s1 };
        Vec3f p1_top{ antRadius * c1, antTopY,    antRadius * s1 };

        Vec3f n0{ c0, 0.0f, s0 };
        Vec3f n1{ c1, 0.0f, s1 };
        Vec3f nAvg{
            0.5f * (n0.x + n1.x),
            0.0f,
            0.5f * (n0.z + n1.z)
        };

        // Quad -> two triangles:
        // Triangle 1: p0_top, p0_bot, p1_bot
        pushTriangleFlat(outPositions, outNormals,
                         p0_top, p0_bot, p1_bot,
                         nAvg);

        // Triangle 2: p0_top, p1_bot, p1_top
        pushTriangleFlat(outPositions, outNormals,
                         p0_top, p1_bot, p1_top,
                         nAvg);
    }

    // Number of vertices we just added (cap + antenna)
    outTopVertexCount = static_cast<int>(outPositions.size()) - outBaseVertexCount;


    // outPositions / outNormals now contain a fairly detailed flying saucer
}
