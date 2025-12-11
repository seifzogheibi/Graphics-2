#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <print>
#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/mat44.hpp"
#include "texture.hpp"

#include "defaults.hpp"
#include "spaceship.hpp"
#include "loadobj.hpp"
#include "camera.hpp"
#include "particles.hpp"

#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include <vector>
#include "ui.hpp"

#define ASSETS "assets/cw2/"

namespace
{
    constexpr char const* kWindowTitle = "COMP3811 - CW2";

    // GLFW callbacks
    void glfw_callback_error_( int, char const* );
    void glfw_callback_key_( GLFWwindow*, int, int, int, int );
    void glfw_callback_mouse_button_( GLFWwindow* , int , int , int );
    void glfw_callback_cursor_pos_( GLFWwindow* , double, double );

    struct GLFWCleanupHelper
    {
        ~GLFWCleanupHelper();
    };
    struct GLFWWindowDeleter
    {
        ~GLFWWindowDeleter();
        GLFWwindow* window;
    };

    // Camera and view modes
    Camera gCamera; // Primary camera
    CameraMode gCameraMode = CameraMode::Free; // main view mode
    bool gSplitScreenEnabled = false;
    CameraMode gCameraMode2 = CameraMode::Chase;  // second view mode

    // Task 1.7 Spaceship animation
    struct VehicleAnim
    {
        bool active = false; // Animation has started at least once
        bool paused = false; // Toggled by F / UI button
        float time   = 0.f; // Seconds since start
    };
    VehicleAnim gUfoAnim;

    // Task 1.10 Particle system
    ParticleSystem gParticleSystem;

    // UI mouse states for buttons (task 1.11)
    double gMouseX= 0.0;
    double gMouseY = 0.0;
    bool gMouseLeftDown = false;

    // Task1.6 Lighting
    struct PointLight
    {
        Vec3f position;
        Vec3f color;
        bool  enabled = true;
    };

    // Three colored point lights
    PointLight gPointLights[3];
    bool gDirectionalLightEnabled = true;

    // Task 1.7 cubic bezier curve
    Vec3f bezier3(
        Vec3f const& A,
        Vec3f const& B,
        Vec3f const& C,
        Vec3f const& D,
        float t
    )
    {
        float it  = 1.f - t;
        float it2 = it * it;
        float t2  = t * t;

        return
            (it2*it) * A +
            (3.f* it2*t)* B +
            (3.f*it* t2) *C +
            (t2*t)* D;
    }

    // Scene rendering (terrain, spaceship, pads, particles)
    void renderScene(
        Mat44f const& viewProj,
        Vec3f const& camPosForLighting,
        GLuint terrainVAO,
        SimpleMeshData const& terrainMeshData,
        GLuint terrainTexture,
        ShaderProgram const& terrainProgram,
        Mat44f const& model,
        Vec3f const& lightDir,
        Vec3f const& ambientColor,
        Vec3f const& baseColor,
        MeshGL const& ufoMesh,
        Mat44f const& ufoModel,
        GLuint landingVao,
        SimpleMeshData const& landingMeshData,
        ShaderProgram const& landingProgram,
        Vec3f const& landingPadPos1,
        Vec3f const& landingPadPos2,
        ShaderProgram const& particleProgram
    )
    {
        Mat44f terrainMvp   = viewProj * model;
        Mat44f ufoMvp       = viewProj * ufoModel;
        Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model)));

        // Prepare point light arrays 
        Vec3f pointLightPositions[3] = {
            gPointLights[0].position,
            gPointLights[1].position,
            gPointLights[2].position
        };
        Vec3f pointLightColorsArr[3] = {
            gPointLights[0].color,
            gPointLights[1].color,
            gPointLights[2].color
        };
        GLint pointLightEnabledArr[3] = {
            gPointLights[0].enabled ? 1 : 0,
            gPointLights[1].enabled ? 1 : 0,
            gPointLights[2].enabled ? 1 : 0
        };

        // Terrain rendering
        GLuint progId = terrainProgram.programId();
        glUseProgram(progId);

        glUniform3fv(2, 1, &lightDir.x);
        glUniform3fv(4, 1, &ambientColor.x);
        glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);
        glUniform3fv(6, 1, &camPosForLighting.x);
        glUniform1i(17, 1); // uUseTexture = 1

        // point lights (positions, colors, enabled)
        glUniform3fv(7, 3, &pointLightPositions[0].x);
        glUniform3fv(10, 3, &pointLightColorsArr[0].x);
        glUniform1iv(13, 3, pointLightEnabledArr);
        glUniform1i(16, gDirectionalLightEnabled ? 1 : 0); // toggles sunlight

        glUniformMatrix4fv(0, 1, GL_TRUE, terrainMvp.v); // uViewProj times model
        glUniformMatrix4fv(18, 1, GL_TRUE, model.v);
        glUniform3fv(3, 1, &baseColor.x);

        // bind terrain texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glUniform1i(5, 0);

        // draw terrain
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)terrainMeshData.positions.size());
        glBindVertexArray(0);

        // ----- UFO -----
        glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);
        glUniformMatrix4fv(18, 1, GL_TRUE, ufoModel.v);
        glBindVertexArray(ufoMesh.vao);
        glUniform1i(17, 0); // uUseTexture = 0

        // Make diffuse/tint colour neutral so vertex colours are used directly
        Vec3f ufoTint{ 1.0f, 1.0f, 1.0f };
        glUniform3fv(3, 1, &ufoTint.x);

        // One MVP for all UFO geometry
        glUniformMatrix4fv(0, 1, GL_TRUE, ufoMvp.v);

        glDrawArrays(GL_TRIANGLES, 0, ufoMesh.vertexCount);

        glBindVertexArray(0);


        // Landing pads rendering
        GLuint landingProgId = landingProgram.programId();
        glUseProgram(landingProgId);

        glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);
        glUniform3fv(2, 1, &lightDir.x);
        glUniform3fv(4, 1, &ambientColor.x);
        glUniform3fv(6, 1, &camPosForLighting.x);
        glUniform3fv(7, 3, &pointLightPositions[0].x);
        glUniform3fv(10, 3, &pointLightColorsArr[0].x);
        glUniform1iv(13, 3, pointLightEnabledArr);
        glUniform1i(16, gDirectionalLightEnabled ? 1 : 0);

        glBindVertexArray(landingVao);

        // first pad
        Mat44f lpModel1 = make_translation(landingPadPos1);
        glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpModel1.v);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)landingMeshData.positions.size());

        // second pad
        Mat44f lpModel2 = make_translation(landingPadPos2);
        glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpModel2.v);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)landingMeshData.positions.size());

        glBindVertexArray(0);

        // Particle rendering
        renderParticles(
            gParticleSystem,
            particleProgram.programId(),
            viewProj.v,
            camPosForLighting
        );
    }

} // namespace


int main() try
{
    // GLFW initialization and window creation
    if( GLFW_TRUE != glfwInit() )
    {
        char const* msg = nullptr;
        int ecode = glfwGetError( &msg );
        throw Error( "glfwInit() failed with '{}' ({})", msg, ecode );
    }

    GLFWCleanupHelper cleanupHelper; 

    glfwSetErrorCallback( &glfw_callback_error_ ); // error callback

    // GLFW window hints
    glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#   if !defined(NDEBUG)
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE ); // debug context
#   endif

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        kWindowTitle,
        nullptr, nullptr
    );

    if( !window ) // window failed
    {
        char const* msg = nullptr;
        int ecode = glfwGetError( &msg );
        throw Error( "glfwCreateWindow() failed with '{}' ({})", msg, ecode );
    }

    GLFWWindowDeleter windowDeleter{ window };

    // Input callbacks
    glfwSetMouseButtonCallback( window, &glfw_callback_mouse_button_ );
    glfwSetCursorPosCallback( window, &glfw_callback_cursor_pos_ );
    glfwSetKeyCallback( window, &glfw_callback_key_ );

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 ); // vsync on

    // GLAD (OpenGL loader)
    if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
        throw Error( "gladLoadGLLoader() failed - cannot load GL API!" );

    std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
    std::print( "VENDOR {}\n",   (char const*)glGetString( GL_VENDOR ) );
    std::print( "VERSION {}\n",  (char const*)glGetString( GL_VERSION ) );
    std::print( "SHADING_LANGUAGE_VERSION {}\n",
                (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

#   if !defined(NDEBUG)
    setup_gl_debug_output();
#   endif

    OGL_CHECKPOINT_ALWAYS();

    // Global GL state
    glEnable(GL_DEPTH_TEST); // depth testing enabled
    glEnable(GL_CULL_FACE); // backface culling enabled
    glEnable(GL_FRAMEBUFFER_SRGB); 
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);

    // point size and blending for particles
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);

    OGL_CHECKPOINT_ALWAYS();

    int iwidth, iheight;
    glfwGetFramebufferSize( window, &iwidth, &iheight );
    glViewport( 0, 0, iwidth, iheight );

    OGL_CHECKPOINT_ALWAYS();

    // terrain mesh loading and shader setup
    SimpleMeshData terrainMeshData = load_wavefront_obj("assets/cw2/parlahti.obj");
    GLuint terrainVAO = create_vao(terrainMeshData);

    ShaderProgram terrainProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/default.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
    });

    // World transform for terrain
    Mat44f model = kIdentity44f;

    // Task1.6 Lighting colors
    Vec3f lightDir = normalize(Vec3f{ 0.f, 1.f, -1.f }); // light direction
    Vec3f baseColor = { 0.6f, 0.7f, 0.6f };
    Vec3f ambientColor = { 0.18f, 0.18f, 0.18f };

    
    // =====================
    // Build UFO once (geometry + VAO) using helper
    // =====================
    UfoMesh ufo = create_ufo_mesh();

    MeshGL ufoMesh          = ufo.mesh;

    float bulbRingY         = ufo.bulbRingY;
    float bulbRadius        = ufo.bulbRadius;
    
    // =====================

    // Load terrain texture
    GLuint terrainTexture =
        load_texture_2d( (ASSETS + terrainMeshData.texture_filepath).c_str() );

    // Landing pad shaders
    ShaderProgram landingProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/landing.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/landing.frag" }
    });

    // Landing pad positions
    Vec3f landingPadPos1{ -11.50f, -0.96f, -54.f };
    Vec3f landingPadPos2{   8.f,   -0.96f,  40.f };

    SimpleMeshData landingMeshData =
    load_wavefront_obj("assets/cw2/landingpad.obj");
    GLuint landingVao = create_vao(landingMeshData);

    // UI setup (task 1.11)
    ShaderProgram uiShader({
        {GL_VERTEX_SHADER,"assets/cw2/ui.vert"},
        {GL_FRAGMENT_SHADER,"assets/cw2/ui.frag"}
    });

    UIRenderer uiRenderer(1280, 720, uiShader); // initial size

    Button launchButton{"Launch", 0, 0, 120, 40,
                        {0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    Button resetButton{"Reset", 0, 0, 120, 40,
                       {0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

    // Point lights attached to the spaceship
    gPointLights[0].color = Vec3f{ 1.f, 0.f, 0.f }; //red
    gPointLights[1].color = Vec3f{ 0.f, 1.f, 0.f }; // green
    gPointLights[2].color = Vec3f{ 0.f, 0.f, 1.f }; // blue

    gPointLights[0].enabled = true;
    gPointLights[1].enabled = true;
    gPointLights[2].enabled = true;
    gDirectionalLightEnabled = true;

    // Particle system shaders and initialization
    ShaderProgram particleProgram({
        {GL_VERTEX_SHADER, "assets/cw2/particle.vert"},
        {GL_FRAGMENT_SHADER, "assets/cw2/particle.frag"}
    });

    initParticleSystem(gParticleSystem, "assets/cw2/particle.png");

    OGL_CHECKPOINT_ALWAYS();

    // Main loop
    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();

        // Framebuffer size and viewport
        float fbwidth, fbheight;
        {
            int nwidth, nheight;
            glfwGetFramebufferSize( window, &nwidth, &nheight );
            fbwidth  = float(nwidth);
            fbheight = float(nheight);

            // pauses if window is minimized
            if( 0 == nwidth || 0 == nheight )
            {
                do
                {
                    glfwWaitEvents();
                    glfwGetFramebufferSize( window, &nwidth, &nheight );
                } while( 0 == nwidth || 0 == nheight );
            }

            glViewport( 0, 0, nwidth, nheight );
        }

        // Time step (dt)
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        if (gUfoAnim.active && !gUfoAnim.paused)
            gUfoAnim.time += dt;

        // Task 1.7 Projection 
        float aspect = fbwidth / fbheight;
        float fovRadians = 60.0f * std::numbers::pi_v<float> / 180.0f;
        float zNear = 0.1f;
        float zFar = 250.0f;

        Mat44f proj = make_perspective_projection(fovRadians, aspect, zNear, zFar);

        // spaceship above first landing pad
        Vec3f ufoStartPos{
            landingPadPos1.x,
            landingPadPos1.y + 1.3f,
            landingPadPos1.z
        };

        float lightRadius = bulbRadius;

        Vec3f lightOffset0{  lightRadius, bulbRingY - 0.35f, 0.0f };
        Vec3f lightOffset1{ -0.5f * lightRadius, bulbRingY - 0.35f,
                             0.866025f * lightRadius };
        Vec3f lightOffset2{ -0.5f * lightRadius, bulbRingY - 0.35f,
                            -0.866025f * lightRadius };

        // inital spaceship before launching (erect on landing pad)
        Vec3f ufoPos    = ufoStartPos;
        Vec3f forwardWS{ 0.f, 1.f, 0.f };
        Vec3f rightWS  { 1.f, 0.f, 0.f };
        Vec3f upWS     { 0.f, 0.f, 1.f };

        float u = 0.0f;

        if (gUfoAnim.active)
        {
            float totalTime = 12.0f; // duration of animation
            float tAnim = gUfoAnim.time;
            if (tAnim < 0.f)       tAnim = 0.f;
            if (tAnim > totalTime) tAnim = totalTime;

            float s = tAnim / totalTime;
            u = s * s;

            float rangeZ    = 140.0f;
            float maxHeight = 80.0f;

            float x0 = ufoStartPos.x;
            float y0 = ufoStartPos.y;
            float z0 = ufoStartPos.z;

            // Bezier controls for curved take off path
            Vec3f A = ufoStartPos;
            Vec3f B{ x0, y0 + maxHeight * 0.7f,z0 };
            Vec3f C{ x0, y0 + maxHeight, z0 + rangeZ * 0.55f };
            Vec3f D{ x0, y0 + maxHeight * 0.2f, z0 + rangeZ };

            ufoPos = bezier3(A, B, C, D, u); // position on curve

            // approximates tangent by sampling slightly ahead on the curve
            float eps = 0.001f;
            float u2 = u + eps;
            if (u2 > 1.0f) u2 = 1.0f;

            Vec3f posAhead = bezier3(A, B, C, D, u2);
            Vec3f vel = posAhead - ufoPos;
            float speed = length(vel);

            if (speed > 1e-4f)
            {
                forwardWS = vel / speed;

                Vec3f worldUp{ 0.f, 1.f, 0.f };
                if (std::fabs(dot(forwardWS, worldUp)) > 0.99f)
                    worldUp = Vec3f{ 1.f, 0.f, 0.f };

                rightWS = normalize(cross(worldUp, forwardWS));
                upWS    = cross(forwardWS, rightWS);
            }
        }
        else
        {
            // space ship pointing upwards on landing pad before launch (idle)
            ufoPos    = ufoStartPos;
            forwardWS = Vec3f{ 0.f, 1.f, 0.f };
            rightWS   = Vec3f{ 1.f, 0.f, 0.f };
            upWS      = Vec3f{ 0.f, 0.f, 1.f };
        }

        // Orient spaceship to follow path direction
        float fy = std::clamp(forwardWS.y, -1.0f, 1.0f);
        float ufoYaw   = std::atan2(forwardWS.x, -forwardWS.z);
        float ufoPitch = std::asin(fy);
        float ufoRoll  = 0.0f;

        Mat44f ufoOrient =
            make_rotation_y(ufoYaw) *
            make_rotation_x(ufoPitch) *
            make_rotation_z(ufoRoll);

        // realigns the spaceships mesh axes to world axes
        Mat44f ufoRot =
            ufoOrient *
            make_rotation_y(std::numbers::pi_v<float>) *
            make_rotation_x(0.5f * std::numbers::pi_v<float>);

        Mat44f ufoModel =
            make_translation(ufoPos) *
            ufoRot *
            make_scaling(0.5f, 0.5f, 0.5f);

            // Rotate the local light offsets by the UFO rotation (no translation)
            Vec4f w0 = ufoRot * Vec4f{ lightOffset0.x, lightOffset0.y, lightOffset0.z, 0.f };
            Vec4f w1 = ufoRot * Vec4f{ lightOffset1.x, lightOffset1.y, lightOffset1.z, 0.f };
            Vec4f w2 = ufoRot * Vec4f{ lightOffset2.x, lightOffset2.y, lightOffset2.z, 0.f };

            lightOffset0 = Vec3f{ w0.x, w0.y, w0.z };
            lightOffset1 = Vec3f{ w1.x, w1.y, w1.z };
            lightOffset2 = Vec3f{ w2.x, w2.y, w2.z };

        // lightOffset0 = ufoModel * lightOffset0;
        // lightOffset1 = ufoModel * lightOffset1;
        // lightOffset2 = ufoModel * lightOffset2;

        // // Attach point lights to the spaceship's body
        gPointLights[0].position = ufoPos + lightOffset0;
        gPointLights[1].position = ufoPos + lightOffset1;
        gPointLights[2].position = ufoPos + lightOffset2;

        // Particle emission and simulation
// At top of main loop scope (inside while, but before any emission logic)
static bool  firstEngineFrame = true;
static Vec3f prevEnginePos{};

// Particle emission and simulation
if (gUfoAnim.active && !gUfoAnim.paused)
{
    Vec3f enginePosCurr = ufoPos - forwardWS * 1.2f;

    if (firstEngineFrame)
    {
        // First frame after (re)starting animation: no history yet
        prevEnginePos   = enginePosCurr;
        firstEngineFrame = false;
    }

    emitParticles(
        gParticleSystem,
        dt,
        prevEnginePos,
        enginePosCurr,
        forwardWS,
        rightWS,
        upWS
    );

    prevEnginePos = enginePosCurr;
}
else
{
    // When animation is paused / reset, reset the "first frame" flag
    firstEngineFrame = true;
}

        if (!gUfoAnim.paused)
        {
            updateParticles(gParticleSystem, dt);
        }

        // Camera movement
        updateCameraMovement(gCamera, dt);

        // Update button positions
        float buttonY = fbheight - 60.0f;
        launchButton.x = fbwidth / 2.0f - 70.0f;
        launchButton.y = buttonY;
        resetButton.x  = fbwidth / 2.0f + 70.0f;
        resetButton.y  = buttonY;

        // Upload alive particles to VBO
        uploadParticleData(gParticleSystem);

        // Begin rendering (draw scene)
        OGL_CHECKPOINT_DEBUG();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!gSplitScreenEnabled)
        {
            // Single fullscreen view
            glViewport(0, 0,
                       (int)fbwidth,
                       (int)fbheight);

            CameraResult camResult = computeCameraView(
                gCameraMode,
                gCamera,
                ufoPos,
                forwardWS,
                landingPadPos1
            );

            Mat44f viewProj = proj * camResult.view;

            renderScene(
                viewProj,
                camResult.position,
                terrainVAO,
                terrainMeshData,
                terrainTexture,
                terrainProgram,
                model,
                lightDir,
                ambientColor,
                baseColor,
                ufoMesh,
                ufoModel,
                landingVao,
                landingMeshData,
                landingProgram,
                landingPadPos1,
                landingPadPos2,
                particleProgram
            );
        }
        else
        {
            // Task 1.9
            // Split screen left and right views
            int leftWidth  = (int)fbwidth / 2;
            int rightWidth = (int)fbwidth - leftWidth;
            int fullHeight = (int)fbheight;

            // Left view (primary camera)
            glViewport(0, 0, leftWidth, fullHeight);
            float aspectLeft = float(leftWidth) / float(fullHeight);
            Mat44f projLeft =
                make_perspective_projection(fovRadians, aspectLeft, zNear, zFar);

            CameraResult camResult1 = computeCameraView(
                gCameraMode,
                gCamera,
                ufoPos,
                forwardWS,
                landingPadPos1
            );

            Mat44f viewProj1 = projLeft * camResult1.view;

            renderScene(
                viewProj1,
                camResult1.position,
                terrainVAO,
                terrainMeshData,
                terrainTexture,
                terrainProgram,
                model,
                lightDir,
                ambientColor,
                baseColor,
                ufoMesh,
                ufoModel,
                landingVao,
                landingMeshData,
                landingProgram,
                landingPadPos1,
                landingPadPos2,
                particleProgram
            );

            // Right view (secondary camera mode)
            glViewport(leftWidth, 0, rightWidth, fullHeight);
            float aspectRight = float(rightWidth) / float(fullHeight);
            Mat44f projRight =
                make_perspective_projection(fovRadians, aspectRight, zNear, zFar);

            CameraResult camResult2 = computeCameraView(
                gCameraMode2,
                gCamera,
                ufoPos,
                forwardWS,
                landingPadPos1
            );

            Mat44f viewProj2 = projRight * camResult2.view;

            renderScene(
                viewProj2,
                camResult2.position,
                terrainVAO,
                terrainMeshData,
                terrainTexture,
                terrainProgram,
                model,
                lightDir,
                ambientColor,
                baseColor,
                ufoMesh,
                ufoModel,
                landingVao,
                landingMeshData,
                landingProgram,
                landingPadPos1,
                landingPadPos2,
                particleProgram
            );

            // Restore full viewport
            glViewport(0, 0, (int)fbwidth, (int)fbheight);
        }

        // Draws UI overlay  (altitude and control buttons)
        uiRenderer.setWindowSize((int)fbwidth, (int)fbheight);
        uiRenderer.beginFrame();

        // Altitude in top left
        char altitudeText[64];
        std::snprintf(altitudeText, sizeof(altitudeText),"Altitude: %.1f m", ufoPos.y);
        uiRenderer.renderText(
            10.0f, 10.0f,
            altitudeText,
            24.0f,
            Vec4f{1.0f, 1.0f, 1.0f, 1.0f}
        );

        // Launch button (start/pause animation)
        if (uiRenderer.renderButton(launchButton, gMouseX, gMouseY, gMouseLeftDown))
        {
            if (!gUfoAnim.active)
            {
                gUfoAnim.active = true;
                gUfoAnim.paused = false;
                gUfoAnim.time = 0.f;
            }
            else
            {
                gUfoAnim.paused = !gUfoAnim.paused;
            }
        }

        // Reset button (stops animation and clears particles)
        if (uiRenderer.renderButton(resetButton, gMouseX, gMouseY, gMouseLeftDown))
        {
            gUfoAnim.active = false;
            gUfoAnim.paused = false;
            gUfoAnim.time = 0.f;

            resetParticles(gParticleSystem);
        }

        uiRenderer.endFrame(); // flush the UI

        glfwSwapBuffers( window );
    }

    return 0;
}
catch( std::exception const& eErr )
{
    std::print( stderr, "Top-level Exception ({}):\n", typeid(eErr).name() );
    std::print( stderr, "{}\n", eErr.what() );
    std::print( stderr, "Bye.\n" );
    return 1;
}


// CALLBACKS
namespace
{
    void glfw_callback_error_( int aErrNum, char const* aErrDesc )
    {
        std::print( stderr, "GLFW error: {} ({})\n", aErrDesc, aErrNum );
    }

    void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
    {
        bool pressed  = (aAction == GLFW_PRESS);

        if (aKey == GLFW_KEY_ESCAPE && pressed)
        {
            glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
            return;
        }

        // Movement controls
        if (aKey == GLFW_KEY_W)
            gCamera.moveForward = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_S)
            gCamera.moveBackward = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_A)
            gCamera.moveLeft = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_D)
            gCamera.moveRight = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_E)
            gCamera.moveUp = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_Q)
            gCamera.moveDown = pressed || (aAction == GLFW_REPEAT);

        // Light toggles
        if (aAction == GLFW_PRESS)
        {
            if (aKey == GLFW_KEY_1)
                gPointLights[0].enabled = !gPointLights[0].enabled;
            else if (aKey == GLFW_KEY_2)
                gPointLights[1].enabled = !gPointLights[1].enabled;
            else if (aKey == GLFW_KEY_3)
                gPointLights[2].enabled = !gPointLights[2].enabled;
            else if (aKey == GLFW_KEY_4)
                gDirectionalLightEnabled = !gDirectionalLightEnabled;
        }

        // Speed modifiers
        if (aKey == GLFW_KEY_LEFT_SHIFT || aKey == GLFW_KEY_RIGHT_SHIFT)
            gCamera.fast = pressed || (aAction == GLFW_REPEAT);
        if (aKey == GLFW_KEY_LEFT_CONTROL || aKey == GLFW_KEY_RIGHT_CONTROL)
            gCamera.slow = pressed || (aAction == GLFW_REPEAT);

        // Spaceship animation controls (keyboard)
        if (aKey == GLFW_KEY_F && aAction == GLFW_PRESS)
        {
            if (!gUfoAnim.active)
            {
                gUfoAnim.active = true;
                gUfoAnim.paused = false;
                gUfoAnim.time   = 0.f;
            }
            else
            {
                gUfoAnim.paused = !gUfoAnim.paused;
            }
        }

        if (aKey == GLFW_KEY_R && aAction == GLFW_PRESS)
        {
            gUfoAnim.active = false;
            gUfoAnim.paused = false;
            gUfoAnim.time   = 0.f;
            resetParticles(gParticleSystem);
        }

        // Toggles splitscreen with V
        if (aKey == GLFW_KEY_V && aAction == GLFW_PRESS)
            gSplitScreenEnabled = !gSplitScreenEnabled;

        // Camera mode cycling (C for left, Shift+C for right)
        if (aKey == GLFW_KEY_C && aAction == GLFW_PRESS)
        {
            bool shiftPressed =
                (glfwGetKey(aWindow, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS ||
                 glfwGetKey(aWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

            if (shiftPressed)
            {
                // Cycle secondary camera mode
                if (gCameraMode2 == CameraMode::Free)
                    gCameraMode2 = CameraMode::Chase;
                else if (gCameraMode2 == CameraMode::Chase)
                    gCameraMode2 = CameraMode::Ground;
                else
                    gCameraMode2 = CameraMode::Free;
            }
            else
            {
                // Cycle main camera mode
                if (gCameraMode == CameraMode::Free)
                    gCameraMode  = CameraMode::Chase;
                else if (gCameraMode == CameraMode::Chase)
                    gCameraMode  = CameraMode::Ground;
                else
                    gCameraMode  = CameraMode::Free;
            }
        }
    }

    void glfw_callback_mouse_button_( GLFWwindow* window, int button, int action, int )
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            gMouseLeftDown = (action == GLFW_PRESS);

        // Right mouse button toggles mouse capture for camera look
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            gCamera.mouseCaptured = !gCamera.mouseCaptured;

            if (gCamera.mouseCaptured)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                gCamera.firstMouse = true;
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    void glfw_callback_cursor_pos_( GLFWwindow*, double xpos, double ypos )
    {
        gMouseX = xpos;
        gMouseY = ypos;

        handleCameraMouseMovement(gCamera, xpos, ypos);
    }

    GLFWCleanupHelper::~GLFWCleanupHelper()
    {
        glfwTerminate();
    }

    GLFWWindowDeleter::~GLFWWindowDeleter()
    {
        if( window )
            glfwDestroyWindow( window );
    }
}
