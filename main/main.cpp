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
#include "ufo.hpp"
#include "loadobj.hpp"
#include "camera.hpp"

#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include <vector>
#include "ui.hpp"

#define ASSETS "assets/cw2/"

namespace
{
    constexpr char const* kWindowTitle = "COMP3811 - CW2";

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

    // ---------- Mesh / shader setup ----------
    struct MeshGL
    {
        GLuint vao = 0;
        GLsizei vertexCount = 0;
    };

    // ---------- Camera / modes ----------
    Camera gCamera;
    CameraMode gCameraMode  = CameraMode::Free;
    bool      gSplitScreenEnabled = false;
    CameraMode gCameraMode2 = CameraMode::Chase;  // Second view's camera mode

    // ---------- UFO animation ----------
    struct VehicleAnim
    {
        bool  active = false; // started at least once
        bool  paused = false; // toggled by F
        float time   = 0.f;   // seconds
    };
    VehicleAnim gUfoAnim;

    // ---------- Particle system ----------
    struct Particle
    {
        Vec3f pos;
        Vec3f vel;
        float life;   // <= 0 => dead
    };

    constexpr int kMaxParticles = 70000;

    Particle gParticles[kMaxParticles];
    int   gAliveCount          = 0;
    float gEmissionAccumulator = 0.0f;
    float gEmissionRate        = 15000.0f;  // particles per second

    GLuint gParticleVAO     = 0;
    GLuint gParticleVBO     = 0;
    GLuint gParticleTexture = 0;

    // UI mouse state (for buttons)
    double gMouseX = 0.0;
    double gMouseY = 0.0;
    bool   gMouseLeftDown = false;

    // ---------- Lighting ----------
    struct PointLight
    {
        Vec3f position;
        Vec3f color;
        bool  enabled = true;
    };

    PointLight gPointLights[3];
    bool gDirectionalLightEnabled = true;

    // particle system helper: find a free particle slot
    int alloc_particle()
    {
        for (int i = 0; i < kMaxParticles; ++i)
        {
            if (gParticles[i].life <= 0.0f)
                return i;
        }
        return -1;
    }

    void reset_particles()
    {
        for (int i = 0; i < kMaxParticles; ++i)
        {
            gParticles[i].life = -1.0f;   // mark all dead
        }
        gAliveCount          = 0;
        gEmissionAccumulator = 0.0f;
    }

    // Simple cubic Bézier in 3D
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
        float t2  = t  * t;

        return
            (it2 * it) * A +
            (3.f * it2 * t) * B +
            (3.f * it  * t2) * C +
            (t2 * t)        * D;
    }

    // ---------- Scene rendering ----------
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
        int ufoBaseVertexCount,
        int ufoTopVertexCount,
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

        // Point light data
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

        // ----- TERRAIN -----
        GLuint progId = terrainProgram.programId();
        glUseProgram(progId);

        glUniform3fv(2, 1, &lightDir.x);
        glUniform3fv(4, 1, &ambientColor.x);
        glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);
        glUniform3fv(6, 1, &camPosForLighting.x);
        glUniform1i(17, 1);  // uUseTexture = 1
        glUniform3fv(7, 3, &pointLightPositions[0].x);
        glUniform3fv(10, 3, &pointLightColorsArr[0].x);
        glUniform1iv(13, 3, pointLightEnabledArr);
        glUniform1i(16, gDirectionalLightEnabled ? 1 : 0);

        glUniformMatrix4fv(0, 1, GL_TRUE, terrainMvp.v);
        glUniformMatrix4fv(18, 1, GL_TRUE, model.v);
        glUniform3fv(3, 1, &baseColor.x);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glUniform1i(5, 0);

        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)terrainMeshData.positions.size());
        glBindVertexArray(0);

        // ----- UFO -----
        glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);
        glUniformMatrix4fv(18, 1, GL_TRUE, ufoModel.v);
        glBindVertexArray(ufoMesh.vao);
        glUniform1i(17, 0);  // uUseTexture = 0

        Vec3f bodyColor{ 0.2f, 0.28f, 0.38f };
        glUniform3fv(3, 1, &bodyColor[0]);
        glUniformMatrix4fv(0, 1, GL_TRUE, ufoMvp.v);
        glDrawArrays(GL_TRIANGLES, 0, ufoBaseVertexCount);

        Vec3f topColor{ 0.25f, 0.45f, 0.95f };
        glUniform3fv(3, 1, &topColor[0]);
        glDrawArrays(GL_TRIANGLES, ufoBaseVertexCount, ufoTopVertexCount);
        glBindVertexArray(0);

        // ----- LANDING PADS -----
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

        Mat44f lpModel1 = make_translation(landingPadPos1);
        glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpModel1.v);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)landingMeshData.positions.size());

        Mat44f lpModel2 = make_translation(landingPadPos2);
        glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpModel2.v);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)landingMeshData.positions.size());

        glBindVertexArray(0);

        // ====================
        // Draw PARTICLES (exhaust)
        // ====================
        if (gAliveCount > 0)
        {
            GLuint particleProgId = particleProgram.programId();
            glUseProgram(particleProgId);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v); // uViewProj
            glUniform1f(1, 6.f);                           // uPointSize
            glUniform3fv(4, 1, &camPosForLighting.x);

            Vec3f exhaustColor{ 0.9f, 0.9f, 1.0f };
            glUniform3fv(2, 1, &exhaustColor.x);          // uColor

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gParticleTexture);
            glUniform1i(3, 0);                            // uTexture

            glBindVertexArray(gParticleVAO);
            glDrawArrays(GL_POINTS, 0, gAliveCount);
            glBindVertexArray(0);

            glDepthMask(GL_TRUE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

} // anonymous namespace


int main() try
{
    // Initialize GLFW
    if( GLFW_TRUE != glfwInit() )
    {
        char const* msg = nullptr;
        int ecode = glfwGetError( &msg );
        throw Error( "glfwInit() failed with '{}' ({})", msg, ecode );
    }

    GLFWCleanupHelper cleanupHelper;

    glfwSetErrorCallback( &glfw_callback_error_ );

    glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#   if !defined(NDEBUG)
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#   endif

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        kWindowTitle,
        nullptr, nullptr
    );

    if( !window )
    {
        char const* msg = nullptr;
        int ecode = glfwGetError( &msg );
        throw Error( "glfwCreateWindow() failed with '{}' ({})", msg, ecode );
    }

    GLFWWindowDeleter windowDeleter{ window };

    glfwSetMouseButtonCallback( window, &glfw_callback_mouse_button_ );
    glfwSetCursorPosCallback( window, &glfw_callback_cursor_pos_ );
    glfwSetKeyCallback( window, &glfw_callback_key_ );

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 ); // V-Sync

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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);

    // For point sprites
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);

    OGL_CHECKPOINT_ALWAYS();

    int iwidth, iheight;
    glfwGetFramebufferSize( window, &iwidth, &iheight );
    glViewport( 0, 0, iwidth, iheight );

    OGL_CHECKPOINT_ALWAYS();

    // Terrain
    SimpleMeshData terrainMeshData = load_wavefront_obj("assets/cw2/parlahti.obj");
    GLuint terrainVAO = create_vao(terrainMeshData);

    ShaderProgram terrainProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/default.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
    });

    // Static transforms + light
    Mat44f model = kIdentity44f;

    Vec3f lightDir     = normalize(Vec3f{ 0.f, 1.f, -1.f });
    Vec3f baseColor    = { 0.6f, 0.7f, 0.6f };
    Vec3f ambientColor = { 0.18f, 0.18f, 0.18f };
    Vec3f diffuseColor = { 0.6f, 0.6f, 0.6f }; // unused

    // Build UFO geometry on CPU
    std::vector<Vec3f> ufoPositions;
    std::vector<Vec3f> ufoNormals;

    int ufoBaseVertexCount = 0;
    int ufoTopVertexCount  = 0;

    int ufoBodyStart   = 0, ufoBodyCount   = 0;
    int ufoEngineStart = 0, ufoEngineCount = 0;
    int ufoFinsStart   = 0, ufoFinsCount   = 0;
    int ufoBulbsStart  = 0, ufoBulbsCount  = 0;
    int ufoTopStart    = 0, ufoTopCount    = 0;

    buildUfoFlatArrays(
        ufoPositions,
        ufoNormals,
        ufoBaseVertexCount,
        ufoTopVertexCount,
        ufoBodyStart,
        ufoBodyCount,
        ufoEngineStart,
        ufoEngineCount,
        ufoFinsStart,
        ufoFinsCount,
        ufoBulbsStart,
        ufoBulbsCount,
        ufoTopStart,
        ufoTopCount
    );

    SimpleMeshData ufoMeshData;
    ufoMeshData.positions = ufoPositions;
    ufoMeshData.normals   = ufoNormals;

    ufoMeshData.Ka.reserve(ufoPositions.size());
    ufoMeshData.Kd.reserve(ufoPositions.size());
    ufoMeshData.Ks.reserve(ufoPositions.size());
    ufoMeshData.Ke.reserve(ufoPositions.size());
    ufoMeshData.Ns.reserve(ufoPositions.size());
    ufoMeshData.colors.reserve(ufoPositions.size());

    for (size_t i = 0; i < ufoPositions.size(); ++i)
    {
        if (i >= (size_t)ufoBulbsStart &&
            i <  (size_t)(ufoBulbsStart + ufoBulbsCount))
        {
            size_t bulbIndex = i - (size_t)ufoBulbsStart;
            size_t groupSize = (size_t)ufoBulbsCount / 3;

            Vec3f Ka, Kd, Ks, Ke;
            if (bulbIndex < groupSize)
            {
                Ka = {0.0f, 0.2f, 0.0f};
                Kd = {0.0f, 0.5f, 0.0f};
                Ks = {0.0f, 0.3f, 0.0f};
                Ke = {0.0f, 1.0f, 0.0f};
            }
            else if (bulbIndex < 2 * groupSize)
            {
                Ka = {0.2f, 0.0f, 0.0f};
                Kd = {0.5f, 0.0f, 0.0f};
                Ks = {0.3f, 0.0f, 0.0f};
                Ke = {1.0f, 0.0f, 0.0f};
            }
            else
            {
                Ka = {0.0f, 0.1f, 0.2f};
                Kd = {0.0f, 0.35f, 0.5f};
                Ks = {0.0f, 0.2f, 0.3f};
                Ke = {0.0f, 0.7f, 1.0f};
            }

            ufoMeshData.Ka.push_back(Ka);
            ufoMeshData.Kd.push_back(Kd);
            ufoMeshData.Ks.push_back(Ks);
            ufoMeshData.Ke.push_back(Ke);
            ufoMeshData.Ns.push_back(32.0f);
            ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
        }
        else if (i >= (size_t)ufoEngineStart &&
                 i <  (size_t)(ufoEngineStart + ufoEngineCount))
        {
            ufoMeshData.Ka.push_back({0.02f, 0.02f, 0.02f});
            ufoMeshData.Kd.push_back({0.80f, 0.80f, 0.80f});
            ufoMeshData.Ks.push_back({1.00f, 1.00f, 1.00f});
            ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
            ufoMeshData.Ns.push_back(512.0f);
            ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
        }
        else if (i >= (size_t)ufoFinsStart &&
                 i <  (size_t)(ufoFinsStart + ufoFinsCount))
        {
            ufoMeshData.Ka.push_back({0.05f, 0.0f, 0.02f});
            ufoMeshData.Kd.push_back({1.00f, 0.00f, 0.80f});
            ufoMeshData.Ks.push_back({0.9f, 0.6f, 0.9f});
            ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
            ufoMeshData.Ns.push_back(96.0f);
            ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
        }
        else if (i >= (size_t)ufoTopStart &&
                 i <  (size_t)(ufoTopStart + ufoTopCount))
        {
            ufoMeshData.Ka.push_back({0.05f, 0.0f, 0.02f});
            ufoMeshData.Kd.push_back({1.00f, 0.00f, 0.80f});
            ufoMeshData.Ks.push_back({0.9f, 0.6f, 0.9f});
            ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
            ufoMeshData.Ns.push_back(96.0f);
            ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
        }
        else
        {
            ufoMeshData.Ka.push_back({0.9f, 0.9f, 0.9f});
            ufoMeshData.Kd.push_back({1.0f, 1.0f, 1.0f});
            ufoMeshData.Ks.push_back({1.0f, 1.0f, 1.0f});
            ufoMeshData.Ke.push_back({0.0f, 0.0f, 0.0f});
            ufoMeshData.Ns.push_back(128.0f);
            ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
        }
    }

    MeshGL ufoMesh;
    GLuint ufoVAO = create_vao(ufoMeshData);
    ufoMesh.vao        = ufoVAO;
    ufoMesh.vertexCount = (GLsizei)ufoPositions.size();

    GLuint terrainTexture =
        load_texture_2d( (ASSETS + terrainMeshData.texture_filepath).c_str() );

    ShaderProgram landingProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/landing.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/landing.frag" }
    });

    // Landing pads
    Vec3f landingPadPos1{ -11.50f, -0.96f, -54.f };
    Vec3f landingPadPos2{  8.f,    -0.96f,  40.f };

    SimpleMeshData landingMeshData =
        load_wavefront_obj("assets/cw2/landingpad.obj");
    GLuint landingVao = create_vao(landingMeshData);

    // UI
    ShaderProgram uiShader({
        { GL_VERTEX_SHADER,   "assets/cw2/ui.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/ui.frag" }
    });

    UIRenderer uiRenderer(1280, 720, uiShader);

    Button launchButton{"Launch", 0, 0, 120, 40,
                        {0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    Button resetButton{"Reset", 0, 0, 120, 40,
                       {0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

    // Point lights
    gPointLights[0].color = Vec3f{ 1.f, 0.f, 0.f };
    gPointLights[1].color = Vec3f{ 0.f, 1.f, 0.f };
    gPointLights[2].color = Vec3f{ 0.f, 0.f, 1.f };

    gPointLights[0].enabled = true;
    gPointLights[1].enabled = true;
    gPointLights[2].enabled = true;
    gDirectionalLightEnabled = true;

    // Particle system shaders
    ShaderProgram particleProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/particle.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/particle.frag" }
    });

    // Allocate particle buffers
    glGenVertexArrays(1, &gParticleVAO);
    glGenBuffers(1,      &gParticleVBO);

    glBindVertexArray(gParticleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gParticleVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        kMaxParticles * sizeof(Vec3f),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vec3f),
        (void*)0
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gParticleTexture = load_texture_2d("assets/cw2/particlecopy.png", false);

    for (int i = 0; i < kMaxParticles; ++i)
        gParticles[i].life = -1.0f;

    gAliveCount          = 0;
    gEmissionAccumulator = 0.0f;

    OGL_CHECKPOINT_ALWAYS();

    // ========== MAIN LOOP ==========
    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();

        float fbwidth, fbheight;
        {
            int nwidth, nheight;
            glfwGetFramebufferSize( window, &nwidth, &nheight );
            fbwidth  = float(nwidth);
            fbheight = float(nheight);

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

        // Time step
        static double lastTime = glfwGetTime();
        double currentTime     = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        if (gUfoAnim.active && !gUfoAnim.paused)
            gUfoAnim.time += dt;

        // Projection
        float aspect     = fbwidth / fbheight;
        float fovRadians = 60.0f * std::numbers::pi_v<float> / 180.0f;
        float zNear      = 0.1f;
        float zFar       = 250.0f;

        Mat44f proj = make_perspective_projection(fovRadians, aspect, zNear, zFar);

        // ===== UFO animation =====
        Vec3f ufoStartPos{
            landingPadPos1.x,
            landingPadPos1.y + 1.6f,
            landingPadPos1.z
        };

        float lightRadius = 0.6f;

        Vec3f lightOffset0{  lightRadius, 0.6f, 0.0f };
        Vec3f lightOffset1{ -0.5f * lightRadius, 0.6f,
                             0.866025f * lightRadius };
        Vec3f lightOffset2{ -0.5f * lightRadius, 0.6f,
                            -0.866025f * lightRadius };

        Vec3f ufoPos    = ufoStartPos;
        Vec3f forwardWS{ 0.f, 1.f, 0.f };
        Vec3f rightWS  { 1.f, 0.f, 0.f };
        Vec3f upWS     { 0.f, 0.f, 1.f };

        float u = 0.0f;

        if (gUfoAnim.active)
        {
            float totalTime = 12.0f;
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

            Vec3f A = ufoStartPos;
            Vec3f B{ x0, y0 + maxHeight * 0.7f,  z0 };
            Vec3f C{ x0, y0 + maxHeight,         z0 + rangeZ * 0.55f };
            Vec3f D{ x0, y0 + maxHeight * 0.2f,  z0 + rangeZ };

            ufoPos = bezier3(A, B, C, D, u);

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
            ufoPos    = ufoStartPos;
            forwardWS = Vec3f{ 0.f, 1.f, 0.f };
            rightWS   = Vec3f{ 1.f, 0.f, 0.f };
            upWS      = Vec3f{ 0.f, 0.f, 1.f };
        }

        float fy = std::clamp(forwardWS.y, -1.0f, 1.0f);
        float ufoYaw   = std::atan2(forwardWS.x, -forwardWS.z);
        float ufoPitch = std::asin(fy);
        float ufoRoll  = 0.0f;

        Mat44f ufoOrient =
            make_rotation_y(ufoYaw) *
            make_rotation_x(ufoPitch) *
            make_rotation_z(ufoRoll);

        Mat44f ufoRot =
            ufoOrient *
            make_rotation_y(std::numbers::pi_v<float>) *
            make_rotation_x(0.5f * std::numbers::pi_v<float>);

        Mat44f ufoModel =
            make_translation(ufoPos) *
            ufoRot *
            make_scaling(0.5f, 0.5f, 0.5f);

        // Attach point lights to UFO
        gPointLights[0].position = ufoPos + lightOffset0;
        gPointLights[1].position = ufoPos + lightOffset1;
        gPointLights[2].position = ufoPos + lightOffset2;

        // --------- Particle simulation ---------
        if (gUfoAnim.active && !gUfoAnim.paused)
        {
            Vec3f enginePos = ufoPos - forwardWS * 1.4f;

            gEmissionAccumulator += gEmissionRate * dt;
            int toSpawn = (int)gEmissionAccumulator;
            if (toSpawn > 0)
                gEmissionAccumulator -= (float)toSpawn;

            Vec3f baseVel = -forwardWS * 9.0f;

            const float spreadRadius   = 0.2f;
            const float verticalSpread = 0.4f;

            for (int n = 0; n < toSpawn; ++n)
            {
                int idx = alloc_particle();
                if (idx < 0)
                    break;

                float u1 = std::rand() / float(RAND_MAX);
                float u2r = std::rand() / float(RAND_MAX);

                float r     = spreadRadius * std::sqrt(u1);
                float theta = 2.0f * std::numbers::pi_v<float> * u2r;

                float dx = r * std::cos(theta);
                float dz = r * std::sin(theta);
                float dy = (std::rand() / float(RAND_MAX) - 0.5f) * verticalSpread;

                Vec3f offset = rightWS * dx + upWS * dz;

                Vec3f jitter{
                    (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f,
                    (std::rand() / float(RAND_MAX) - 0.5f) * 3.0f,
                    (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f
                };

                Vec3f vel = baseVel + jitter;

                float tFrac = std::rand() / float(RAND_MAX);
                Vec3f substepOffset = vel * (tFrac * dt);

                gParticles[idx].pos  = enginePos + offset + substepOffset;
                gParticles[idx].vel  = vel;
                gParticles[idx].life = 1.f;
            }
        }

        if (!gUfoAnim.paused)
        {
            for (int i = 0; i < kMaxParticles; ++i)
            {
                if (gParticles[i].life > 0.0f)
                {
                    gParticles[i].life -= dt;
                    if (gParticles[i].life > 0.0f)
                    {
                        gParticles[i].pos =
                            gParticles[i].pos + gParticles[i].vel * dt;

                        if (gParticles[i].pos.y < -0.98f)
                            gParticles[i].life = 0.0f;
                    }
                }
            }
        }

        // Camera movement (WASD etc.)
        updateCameraMovement(gCamera, dt);

        // Update button positions (bottom centre)
        float buttonY = fbheight - 60.0f;
        launchButton.x = fbwidth / 2.0f - 70.0f;
        launchButton.y = buttonY;
        resetButton.x  = fbwidth / 2.0f + 70.0f;
        resetButton.y  = buttonY;

        // Upload alive particles to VBO
        static Vec3f particlePositions[kMaxParticles];
        int alive = 0;
        for (int i = 0; i < kMaxParticles; ++i)
        {
            if (gParticles[i].life > 0.0f)
                particlePositions[alive++] = gParticles[i].pos;
        }
        gAliveCount = alive;

        if (gAliveCount > 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, gParticleVBO);
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                gAliveCount * sizeof(Vec3f),
                particlePositions
            );
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // ===== DRAW =====
        OGL_CHECKPOINT_DEBUG();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!gSplitScreenEnabled)
        {
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
                ufoBaseVertexCount,
                ufoTopVertexCount,
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
            int leftWidth  = (int)fbwidth / 2;
            int rightWidth = (int)fbwidth - leftWidth;
            int fullHeight = (int)fbheight;

            // Left view
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
                ufoBaseVertexCount,
                ufoTopVertexCount,
                ufoModel,
                landingVao,
                landingMeshData,
                landingProgram,
                landingPadPos1,
                landingPadPos2,
                particleProgram
            );

            // Right view
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
                ufoBaseVertexCount,
                ufoTopVertexCount,
                ufoModel,
                landingVao,
                landingMeshData,
                landingProgram,
                landingPadPos1,
                landingPadPos2,
                particleProgram
            );

            glViewport(0, 0, (int)fbwidth, (int)fbheight);
        }

        // ----- UI -----
        uiRenderer.setWindowSize((int)fbwidth, (int)fbheight);
        uiRenderer.beginFrame();

        char altitudeText[64];
        std::snprintf(altitudeText, sizeof(altitudeText),
                      "Altitude: %.1f m", ufoPos.y);
        uiRenderer.renderText(
            10.0f, 10.0f,
            altitudeText,
            24.0f,
            Vec4f{1.0f, 1.0f, 1.0f, 1.0f}
        );

        if (uiRenderer.renderButton(launchButton, gMouseX, gMouseY, gMouseLeftDown))
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

        if (uiRenderer.renderButton(resetButton, gMouseX, gMouseY, gMouseLeftDown))
        {
            gUfoAnim.active = false;
            gUfoAnim.paused = false;
            gUfoAnim.time   = 0.f;

            reset_particles();
        }

        uiRenderer.endFrame();

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


// ======================= CALLBACKS =======================
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

        // UFO animation controls
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
            reset_particles();
        }

        // Toggle split screen with V
        if (aKey == GLFW_KEY_V && aAction == GLFW_PRESS)
            gSplitScreenEnabled = !gSplitScreenEnabled;

        // Camera mode cycling (Shift+C for right view)
        if (aKey == GLFW_KEY_C && aAction == GLFW_PRESS)
        {
            bool shiftPressed =
                (glfwGetKey(aWindow, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS ||
                 glfwGetKey(aWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

            if (shiftPressed)
            {
                if (gCameraMode2 == CameraMode::Free)
                    gCameraMode2 = CameraMode::Chase;
                else if (gCameraMode2 == CameraMode::Chase)
                    gCameraMode2 = CameraMode::Ground;
                else
                    gCameraMode2 = CameraMode::Free;
            }
            else
            {
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
