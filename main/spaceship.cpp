#include "spaceship.hpp"
#include "shapes.hpp"

#include <vector>
#include <cmath>
#include <numbers>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

spaceshipMesh create_spaceship_mesh()
{
    // primary color/body material
    Vec3f KaBody{0.1f, 0.1f, 0.1f};
    Vec3f KdBody{0.9f, 0.9f, 0.9f};
    Vec3f KeBody{0.0f, 0.0f, 0.0f};
    Vec3f KsBody{0.8f, 0.8f, 0.8f};
    float NsBody = 64.0f;

    // secondary color material
    Vec3f KaPink{0.05f, 0.0f, 0.02f};
    Vec3f KdPink{1.0f, 0.0f, 0.8f};
    Vec3f KePink{0.0f, 0.0f, 0.0f};
    Vec3f KsPink{0.9f, 0.6f, 0.9f};
    float NsPink = 32.f;

    // engine material
    Vec3f KaEngine{0.05f, 0.05f, 0.06f};
    Vec3f KdEngine{0.77f, 0.77f, 0.77f};
    Vec3f KeEngine{0.0f, 0.0f, 0.0f};
    Vec3f KsEngine{1.0f, 1.0f, 1.0f};
    float NsEngine = 256.0f;

    // vertex color to be stored 
    Vec3f white{1.f, 1.f, 1.f};
    
    // BODY
    // size in local spaceship space
    float bodyHeight = 5.0f;
    float bodyRadius = 0.4f;
    float engineHeight = 0.8f;
    float engineRadius = bodyRadius * 1.5f;
    float bodyBottomY = -bodyHeight * 0.5f;
    float bodyTopY = bodyHeight * 0.5f;
    float const halfBodyHeight = bodyHeight * 0.5f;

    // body pre-transform
    Mat44f bodyPre =
        // move cylinder so that it centers around y=0 after scaling
        make_translation(Vec3f{0.f, -halfBodyHeight, 0.f}) *
        // rotate cylinder 90deg about z, since make_cylinder creates it on +x and the cylinder in this case must be upright (+y)
        make_rotation_z(0.5f * std::numbers::pi_v<float>) *
        // scale x to body height, y/z to body radius
        make_scaling(bodyHeight, bodyRadius, bodyRadius);

    // build body
    SimpleMeshData bodyMesh = make_cylinder(
        true,
        60,
        white,
        bodyPre,
        NsBody, KaBody, KdBody, KeBody, KsBody
    );

    // ENGINE
    // place at the bottom of the cylinder
    float engineCenterY = bodyBottomY + engineHeight * 0.5f;
    float const halfEngineHeight = engineHeight * 0.5f;

    Mat44f enginePre =
        make_translation(Vec3f{0.f, engineCenterY - halfEngineHeight, 0.f}) *
        make_rotation_z(0.5f * std::numbers::pi_v<float>) *
        make_scaling(engineHeight, engineRadius, engineRadius);

    SimpleMeshData engineMesh = make_cone(
        true,
        48,
        white,
        enginePre,
        NsEngine, KaEngine, KdEngine, KeEngine, KsEngine
    );

    // BULBS

    float bulbsHeight = 0.7f;
    // positions for bulbs around the cylinder
    float angle0 = 0.0f;
    float angle120 = 2.0f * std::numbers::pi_v<float> / 3.0f;
    float angle240 = 4.0f * std::numbers::pi_v<float> / 3.0f;

    // red bulb pre-transform
    // position around y, translate outwards, then scale to smaller size
    Mat44f redPre =
        make_rotation_y(angle0) *
        make_translation(Vec3f{ bodyRadius, bulbsHeight, 0.0f }) * 
        make_scaling(0.1f, 0.1f, 0.1f);

    SimpleMeshData redB = make_cube(
        true,
        1,
        white,
        redPre,
        // same properties as engine works
        NsEngine,
        KaEngine,
        // change kd to red
        Vec3f {1.f, 0.f, 0.f},
        KeEngine,
        KsEngine
    );

    // green bulb
    Mat44f greenPre =
    make_rotation_y(angle240) *
        make_translation(Vec3f{ bodyRadius, bulbsHeight, 0.0f }) *
        make_scaling(0.1f, 0.1f, 0.1f);

    SimpleMeshData greenB = make_cube(
        true,
        1,
        white,
        greenPre,
        NsEngine,
        KaEngine,
        // green
        Vec3f {0.f, 1.f, 0.f},
        KeEngine,
        KsEngine
    );

    // blue bulb
    Mat44f bluePre =
        make_rotation_y(angle120) *
        make_translation(Vec3f{ bodyRadius, bulbsHeight, 0.0f }) *
        make_scaling(0.1f, 0.1f, 0.1f);

    SimpleMeshData blueB= make_cube(
        true,
        1,
        white,
        bluePre,
        NsEngine,
        KaEngine,
        // blue
        Vec3f {0.f, 0.65f, 1.f},
        KeEngine,
        KsEngine
    );

    // FINS
    float finHeight = 1.2f;
    float finLength = 1.0f;
    float finThickness = .3f;
    float finBaseY = bodyBottomY + 0.4f;
    float finRadius = 0.4f;

    // fin 1 pre transform
    Mat44f finPre =
        make_rotation_y(angle0) *
        make_translation(Vec3f{finRadius, finBaseY, 0.0f}) *
        make_scaling(finLength, finHeight, finThickness);

    SimpleMeshData finMesh = make_fin(
        true,
        16,
        white,
        finPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // fin 2 
    Mat44f finPre2 =
        make_rotation_y(angle120) *
        make_translation(Vec3f{finRadius, finBaseY, 0.0f}) *
        make_scaling(finLength, finHeight, finThickness);

    SimpleMeshData finMesh2 = make_fin(
        true,
        16,
        white,
        finPre2,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // fin 3
    Mat44f finPre3 =
        make_rotation_y(angle240) *
        make_translation(Vec3f{finRadius, finBaseY, 0.0f}) *
        make_scaling(finLength, finHeight, finThickness);

    SimpleMeshData finMesh3 = make_fin(
        true,
        16,
        white,
        finPre3,
        NsPink, KaPink, KdPink, KePink, KsPink
    );


    // combine iniial body parts to base cylinder (bulbs, fins, engine)
    SimpleMeshData baseMesh = concatenate(bodyMesh, engineMesh);
    baseMesh = concatenate( baseMesh, redB);
    baseMesh = concatenate(baseMesh, greenB);
    baseMesh = concatenate(baseMesh, blueB);
    baseMesh = concatenate(baseMesh, finMesh);
    baseMesh = concatenate(baseMesh, finMesh2);
    baseMesh = concatenate(baseMesh, finMesh3);

    // UPPER SPACESHIP
    // NECK
    float neckHeight = 0.5f;
    float neckRadius = bodyRadius;
    float neckCenterY = bodyTopY + neckHeight * 0.5f;
    float const halfNeckHeight = neckHeight * 0.5f;

Mat44f neckPre =
    make_translation(Vec3f{0.f, neckCenterY - halfNeckHeight, 0.f}) *
    make_rotation_z(0.5f * std::numbers::pi_v<float>) *
    make_scaling(neckHeight, neckRadius, neckRadius);

    SimpleMeshData neckMesh = make_cylinder(
        true,
        48,
        white,
        neckPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // TOP CONE
    float coneHeight = 2.f;
    float coneRadius = bodyRadius;
    float coneCenterY = bodyTopY + neckHeight + coneHeight * 0.5f;
    float const halfConeHeight = coneHeight * 0.5f;

    Mat44f conePre =
        make_translation(Vec3f{0.f, coneCenterY - halfConeHeight, 0.f}) *
        make_rotation_z(0.5f * std::numbers::pi_v<float>) *
        make_scaling(coneHeight, coneRadius, coneRadius);

    SimpleMeshData coneMesh = make_cone(
        true,
        48,
        white,
        conePre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // ANTENNA
    float antennaHeight = 0.5f;
    float antennaRadius = 0.05f;
    float antennaCenterY = bodyTopY + neckHeight + coneHeight - antennaHeight * 0.5f;

    float const halfAntennaHeight = antennaHeight * 0.5f;

    Mat44f antennaPre =
        make_translation(Vec3f{0.f, antennaCenterY - halfAntennaHeight, 0.f}) *
        make_rotation_z(0.5f * std::numbers::pi_v<float>) *
        make_scaling(antennaHeight, antennaRadius, antennaRadius);


    SimpleMeshData antennaMesh = make_cylinder(
        true,
        16,
        white,
        antennaPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // TIP
    float tipHeight = 0.3f;
    float tipRadius = antennaRadius;
    float tipCenterY = antennaCenterY + 0.5f * (antennaHeight + tipHeight);
    float const halfTipHeight = tipHeight * 0.5f;

    Mat44f tipPre =
        make_translation(Vec3f{0.f, tipCenterY - halfTipHeight, 0.f}) *
        make_rotation_z(0.5f * std::numbers::pi_v<float>) *
        make_scaling(tipHeight, tipRadius, tipRadius);

    SimpleMeshData tipMesh = make_cone(
        true,
        16,
        Vec3f{1.f, 0.75f, 0.8f}, 
        tipPre,
        NsPink, KaPink, KdPink, KePink, KsPink
    );

    // combine upper spaceship (neck, antenna, tip)
    SimpleMeshData topMesh = concatenate(neckMesh, coneMesh);
    topMesh = concatenate(topMesh, antennaMesh);
    topMesh = concatenate(topMesh, tipMesh);

    // combine body with upper spaceship
    SimpleMeshData spaceshipMeshData = concatenate(baseMesh, topMesh);

    // use a vao to upload mesh
    GLuint spaceshipVAO = create_vao(spaceshipMeshData);
    GLsizei spaceshipVertexCount = (GLsizei)spaceshipMeshData.positions.size();

    return spaceshipMesh{
        spaceshipVAO,
        spaceshipVertexCount,
        bulbsHeight,
        bodyRadius
    };
}
