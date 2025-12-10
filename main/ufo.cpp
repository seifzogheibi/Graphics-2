// ufo.cpp

#include "ufo.hpp"
#include "shapes.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

UfoMesh create_ufo_mesh()
{
    // =====================
    // Build UFO using SimpleMeshData + pre-transform matrices
    // =====================

    // Common material values
    Vec3f KaBody{0.1f, 0.1f, 0.1f};
    Vec3f KdBody{0.9f, 0.9f, 0.9f};
    Vec3f KeBody{0.0f, 0.0f, 0.0f};
    Vec3f KsBody{0.8f, 0.8f, 0.8f};
    Vec3f NsBody{64.0f, 0.f, 0.f};   // shininess in x

    Vec3f KaPink{0.05f, 0.0f, 0.02f};
    Vec3f KdPink{1.0f, 0.0f, 0.8f};
    Vec3f KePink{0.0f, 0.0f, 0.0f};
    Vec3f KsPink{0.9f, 0.6f, 0.9f};
    Vec3f NsPink{96.f, 0.f, 0.f};

    Vec3f KaEngine{0.05f, 0.05f, 0.06f};      // subtle cool metal tint
    Vec3f KdEngine{0.77f, 0.77f, 0.77f};      // ALMOST no diffuse
    Vec3f KeEngine{0.0f, 0.0f, 0.0f};
    Vec3f KsEngine{1.0f, 1.0f, 1.0f};         // perfect mirror specular
    Vec3f NsEngine{256.0f, 0.f, 0.f};            // very shiny

    Vec3f white{1.f, 1.f, 1.f};

    // ----- Dimensions in local UFO space -----
    float bodyHeight   = 5.0f;
    float bodyRadius   = 0.4f;
    float engineHeight = 0.8f;
    float engineRadius = bodyRadius * 1.5f;

    float bodyBottomY = -bodyHeight * 0.5f; // -3
    float bodyTopY    =  bodyHeight * 0.5f; // +3

    // =====================
    // BASE MESH (body + exhaust cone + bulbs)
    // =====================

    // Body: cylinder along local Y, scaled to height 6 and radius 0.4
    Mat44f bodyPre =
        make_scaling(bodyRadius * 2.f,  // x: desired radius from unit radius=0.5
                     bodyHeight,        // y: height from unit [-0.5,0.5]
                     bodyRadius * 2.f);

    SimpleMeshData bodyMesh = make_cylinder(
        true,          // capped
        100,            // subdivisions
        white,         // vertex color
        bodyPre,
        NsBody, KaBody, KdBody, KeBody, KsBody
    );

    // Exhaust: cone at bottom, flared out
    float engineCenterY = bodyBottomY + engineHeight * 0.5f;
    Mat44f enginePre =
        make_translation(Vec3f{0.f, engineCenterY, 0.f}) *
        make_scaling(engineRadius * 2.f,
                     engineHeight,
                     engineRadius * 2.f);

    SimpleMeshData engineMesh = make_cone(
        true,
        48,
        white,
        enginePre,
        NsEngine, KaEngine, KdEngine, KeEngine, KsEngine
    );

    // Bulbs: three tiny cylinders around a ring
    // Common scale for the “bulb” cubes

float bulbRingY   = 0.7f;                 // somewhere around mid-body
float bulbRadius  = bodyRadius;    // just outside the hull

Mat44f lightScale = make_scaling(0.1f, 0.1f, 0.1f);

// Angles for 3 bulbs (0°, 120°, 240°)
float angle0 = 0.0f;
float angle4 = 4.0f * std::numbers::pi_v<float> / 3.0f;
float angle12 = 2.0f * std::numbers::pi_v<float> / 3.0f;

// Red light cube
Mat44f redPre =
    make_rotation_y(angle0) *
    make_translation(Vec3f{ bulbRadius, bulbRingY, 0.0f }) *
    lightScale;

SimpleMeshData redLightCube = make_cube(
    true,
    1,
    Vec3f{1.f, 0.f, 0.f},
    redPre,
    NsEngine,
    KaEngine,
    Vec3f {1.f, 0.f, 0.f},
    KeEngine,
    KsEngine
);

// Green light cube
Mat44f greenPre =
 make_rotation_y(angle4) *
    make_translation(Vec3f{ bulbRadius, bulbRingY, 0.0f }) *
    lightScale;

SimpleMeshData greenLightCube = make_cube(
    true,
    1,
    Vec3f{0.f, 1.f, 0.f},
    greenPre,
    NsEngine,
    KaEngine,
    Vec3f {0.f, 1.f, 0.f},
    KeEngine,
    KsEngine
);

// Blue light cube
Mat44f bluePre =
    make_rotation_y(angle12) *
    make_translation(Vec3f{ bulbRadius, bulbRingY, 0.0f }) *
    lightScale;

SimpleMeshData blueLightCube = make_cube(
    true,
    1,
    Vec3f{0.f, 0.f, 1.f},
    bluePre,
    NsEngine,
    KaEngine,
    Vec3f {0.f, 0.65f, 1.f},
    KeEngine,
    KsEngine
);


        // =====================
    // FINS (3 right triangles evenly spaced around body)
    // =====================

    float finHeight   = 1.2f;     // vertical size
    float finLength   = 1.0f;     // how far it sticks out
    float finThickness = .3f;
    float finBaseY    = bodyBottomY + 0.4f; // vertical position of the base

    float finRadius = 0.4f;            // distance from centre (you requested 0.4f)
    float twoPi = 2.0f * std::numbers::pi_v<float>;

    // Angle step for 3 fins
    float angleStep = twoPi / 3.0f;

    // Fin 0
    Mat44f finPre0 =
        make_rotation_y(0.0f) *
        make_translation(Vec3f{finRadius, finBaseY, 0.0f}) *
        make_scaling(finLength, finHeight, finThickness);

    SimpleMeshData finMesh0 = make_fin(
        true,
        16,
        white,
        finPre0,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // Fin 1 (rotated 120 degrees)
    float angle1 = angleStep;
    Mat44f finPre1 =
        make_rotation_y(angle1) *
        make_translation(Vec3f{finRadius, finBaseY, 0.0f}) *
        make_scaling(finLength, finHeight, finThickness);

    SimpleMeshData finMesh1 = make_fin(
        true,
        16,
        white,
        finPre1,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // Fin 2 (rotated 240 degrees)
    float angle2 = 2.0f * angleStep;
    Mat44f finPre2 =
        make_rotation_y(angle2) *
        make_translation(Vec3f{finRadius, finBaseY, 0.0f}) *
        make_scaling(finLength, finHeight, finThickness);

    SimpleMeshData finMesh2 = make_fin(
        true,
        16,
        white,
        finPre2,
        NsPink, KaPink, KdPink, KePink, KsPink
    );


    // Concatenate all base parts
    SimpleMeshData baseMesh = concatenate(bodyMesh, engineMesh);
    baseMesh = concatenate( baseMesh, redLightCube);
    baseMesh = concatenate(baseMesh, greenLightCube);
    baseMesh = concatenate(baseMesh, blueLightCube);


     // Add fins
    baseMesh = concatenate(baseMesh, finMesh0);
    baseMesh = concatenate(baseMesh, finMesh1);
    baseMesh = concatenate(baseMesh, finMesh2);

    // =====================
    // TOP MESH (neck + big pink cone + antenna + tip)
    // =====================

    float neckHeight = 0.5f;
    float neckRadius = bodyRadius;
    float neckCenterY = bodyTopY + neckHeight * 0.5f;

    Mat44f neckPre =
        make_translation(Vec3f{0.f, neckCenterY, 0.f}) *
        make_scaling(neckRadius * 2.f,
                     neckHeight,
                     neckRadius * 2.f);

    SimpleMeshData neckMesh = make_cylinder(
        true,
        32,
        Vec3f{1.f, 0.75f, 0.8f},  // hot pink
        neckPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // Big pink cone under antenna
    float coneHeight = 2.f;
    float coneRadius = bodyRadius;
    float coneCenterY = bodyTopY + neckHeight + coneHeight * 0.5f;

    Mat44f conePre =
        make_translation(Vec3f{0.f, coneCenterY, 0.f}) *
        make_scaling(coneRadius * 2.f,
                     coneHeight,
                     coneRadius * 2.f);

    SimpleMeshData coneMesh = make_cone(
        true,
        48,
        Vec3f{1.0f,0.75f, 0.8f},  // pink colour
        conePre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // Thin antenna cylinder on top
    float antennaHeight = 0.5f;
    float antennaRadius = 0.05f;
    float antennaCenterY =
        bodyTopY + neckHeight + coneHeight - antennaHeight * 0.5f;

    Mat44f antennaPre =
        make_translation(Vec3f{0.f, antennaCenterY, 0.f}) *
        make_scaling(antennaRadius * 2.f,
                     antennaHeight,
                     antennaRadius * 2.f);

    SimpleMeshData antennaMesh = make_cylinder(
        true,
        16,
        Vec3f{1.0f,0.75f, 0.8f},
        antennaPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // Tiny tip cone at very top
    float tipHeight = 0.3f;
    float tipRadius = antennaRadius;
    float tipCenterY = antennaCenterY + 0.5f * (antennaHeight + tipHeight);

    Mat44f tipPre =
        make_translation(Vec3f{0.f, tipCenterY, 0.f}) *
        make_scaling(tipRadius * 2.f,
                     tipHeight,
                     tipRadius * 2.f);

    SimpleMeshData tipMesh = make_cone(
        true,
        16,
        Vec3f{1.f, 0.75f, 0.8f}, 
        tipPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    SimpleMeshData topMesh = concatenate(neckMesh, coneMesh);
    topMesh = concatenate(topMesh, antennaMesh);
    topMesh = concatenate(topMesh, tipMesh);


    // =====================
    // FINAL UFO MESH + COUNTS
    // =====================

    SimpleMeshData ufoMeshData = concatenate(baseMesh, topMesh);

    // // Create VAO for the spaceship
    MeshGL ufoMesh;
    GLuint ufoVAO = create_vao(ufoMeshData);
    ufoMesh.vao         = ufoVAO;
    ufoMesh.vertexCount = (GLsizei)ufoMeshData.positions.size();

    return UfoMesh{
        ufoMesh,
        bulbRingY,
        bulbRadius
    };
}
