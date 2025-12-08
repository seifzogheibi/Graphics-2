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



    Camera gCamera;
    CameraMode gCameraMode = CameraMode::Free;
    // Task 1.9
    // Split screen state
    bool gSplitScreenEnabled = false;
    CameraMode gCameraMode2 = CameraMode::Chase;  // Second view's camera mode

    // 1.7
    struct VehicleAnim
    {
        bool active = false;   // animation has been started at least once
        bool paused = false;   // toggled by F
        float time = 0.f;      // seconds since animation start
    };

    VehicleAnim gUfoAnim;

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
    gAliveCount = 0;
    gEmissionAccumulator = 0.0f;
}



    // Simple cubic Bézier in 3D (not actually used in this version,
    // but kept in case needed)
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



    struct PointLight
    {
        Vec3f position;
        Vec3f color;
        bool enabled = true;
    };

    PointLight gPointLights[3];
    bool gDirectionalLightEnabled = true;


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
        Vec3f const& landingPadPos2
    )
    {
        Mat44f terrainMvp = viewProj * model;
        Mat44f ufoMvp = viewProj * ufoModel;
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
        glDrawArrays(GL_TRIANGLES, 0, terrainMeshData.positions.size());
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
        glDrawArrays(GL_TRIANGLES, 0, landingMeshData.positions.size());

        Mat44f lpModel2 = make_translation(landingPadPos2);
        glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpModel2.v);
        glDrawArrays(GL_TRIANGLES, 0, landingMeshData.positions.size());

        glBindVertexArray(0);

        // ====================
        // Draw PARTICLES (exhaust)
        // ====================
        if (gAliveCount > 0)
        {
            GLuint particleProgId = particleProgram.programId();
            glUseProgram(particleProgId);

            // Additive blending for exhaust
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // override default
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Keep depth test, but disable depth writes so particles don't overwrite each other
            glDepthMask(GL_FALSE);

            // uViewProj at location 0
            glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);

            // uPointSize at location 1 (pixels)
            // glUniform1f(1, 16.0f);   // tweak 24–48 so each particle covers multiple pixels
            glUniform1f(1, 6.f);              // tweak this value
            glUniform3fv(4, 1, &camPosForLighting.x);

            // uColor at location 2 (tint)
            Vec3f exhaustColor{ 0.9f, 0.9f, 1.0f }; // bluish-white exhaust
            glUniform3fv(2, 1, &exhaustColor.x);

            // uTexture at location 3
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gParticleTexture);
            glUniform1i(3, 0);

            glBindVertexArray(gParticleVAO);
            glDrawArrays(GL_POINTS, 0, gAliveCount);
            glBindVertexArray(0);

            // Restore state for any later draws
            glDepthMask(GL_TRUE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        // 1.9 END
    }

    // Forward declarations for extra callbacks
    void glfw_callback_mouse_button_( GLFWwindow* , int , int , int );
    void glfw_callback_cursor_pos_( GLFWwindow* , double, double );
}

int main() try
{
    // Initialize GLFW
    if( GLFW_TRUE != glfwInit() )
    {
        char const* msg = nullptr;
        int ecode = glfwGetError( &msg );
        throw Error( "glfwInit() failed with '{}' ({})", msg, ecode );
    }

    // Ensure that we call glfwTerminate() at the end of the program.
    GLFWCleanupHelper cleanupHelper;

    // Configure GLFW and create window
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

    // Set up event handling
    glfwSetMouseButtonCallback( window, &glfw_callback_mouse_button_ );
    glfwSetCursorPosCallback( window, &glfw_callback_cursor_pos_ );
    glfwSetKeyCallback( window, &glfw_callback_key_ );

    // Set up drawing stuff
    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 ); // V-Sync is on.

    // Initialize GLAD
    if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
        throw Error( "gladLoadGLLoader() failed - cannot load GL API!" );

    std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
    std::print( "VENDOR {}\n", (char const*)glGetString( GL_VENDOR ) );
    std::print( "VERSION {}\n", (char const*)glGetString( GL_VERSION ) );
    std::print( "SHADING_LANGUAGE_VERSION {}\n", (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

#   if !defined(NDEBUG)
    setup_gl_debug_output();
#   endif

    OGL_CHECKPOINT_ALWAYS();

    // Global GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);

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
    Vec3f diffuseColor = { 0.6f, 0.6f, 0.6f }; // (unused but fine)

    // Build UFO geometry on CPU
    std::vector<Vec3f> ufoPositions;
    std::vector<Vec3f> ufoNormals;

	// counts for base (all bottom parts) and top (nose + antenna)
	int ufoBaseVertexCount = 0;
	int ufoTopVertexCount  = 0;

	// sub-part ranges
	int ufoBodyStart   = 0, ufoBodyCount   = 0;
	int ufoEngineStart = 0, ufoEngineCount = 0;
	int ufoFinsStart   = 0, ufoFinsCount   = 0;
	int ufoBulbsStart  = 0, ufoBulbsCount  = 0;
	int ufoTopStart    = 0, ufoTopCount    = 0;

	// Call buildUfoFlatArrays FIRST to populate the vectors
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

	// NOW create SimpleMeshData and assign the populated vectors
	SimpleMeshData ufoMeshData;
	ufoMeshData.positions = ufoPositions;
	ufoMeshData.normals = ufoNormals;


	 ufoMeshData.Ka.reserve(ufoPositions.size());
		ufoMeshData.Kd.reserve(ufoPositions.size());
		ufoMeshData.Ks.reserve(ufoPositions.size());
		ufoMeshData.Ke.reserve(ufoPositions.size());
		ufoMeshData.Ns.reserve(ufoPositions.size());
		ufoMeshData.colors.reserve(ufoPositions.size());  // Need colors too!
	
		for (size_t i = 0; i < ufoPositions.size(); ++i)
		{
			// --- Bulbs: emissive RGB ---
			if (i >= static_cast<size_t>(ufoBulbsStart) &&
				i <  static_cast<size_t>(ufoBulbsStart + ufoBulbsCount))
			{
				// Divide bulbs into 3 equal groups (one per box)
				size_t bulbIndex = i - static_cast<size_t>(ufoBulbsStart);
				size_t groupSize = ufoBulbsCount / 3;
	
				Vec3f Ka, Kd, Ks, Ke;
				if (bulbIndex < groupSize)
				{
					// Green bulb
					Ka = {0.0f, 0.2f, 0.0f};
					Kd = {0.0f, 0.5f, 0.0f};
					Ks = {0.0f, 0.3f, 0.0f};
					Ke = {0.0f, 1.0f, 0.0f};
				}
				else if (bulbIndex < 2 * groupSize)
				{
					// Red bulb
					Ka = {0.2f, 0.0f, 0.0f};
					Kd = {0.5f, 0.0f, 0.0f};
					Ks = {0.3f, 0.0f, 0.0f};
					Ke = {1.0f, 0.0f, 0.0f};
				}
				
				else
				{
					// Blue bulb
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
			// --- Engine / booster: metallic silver ---
			else if (i >= static_cast<size_t>(ufoEngineStart) &&
					 i <  static_cast<size_t>(ufoEngineStart + ufoEngineCount))
			{ufoMeshData.Ka.push_back({0.10f, 0.10f, 0.10f});
				ufoMeshData.Kd.push_back({0.70f, 0.70f, 0.70f});
				ufoMeshData.Ks.push_back({1.00f, 1.00f, 1.00f});  // very shiny
				ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
				ufoMeshData.Ns.push_back(256.0f);                 // very tight highlight
				ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
				// ufoMeshData.Ka.push_back({0.30f, 0.30f, 0.30f});
				// ufoMeshData.Kd.push_back({0.70f, 0.70f, 0.70f});
				// ufoMeshData.Ks.push_back({1.00f, 1.00f, 1.00f});
				// ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
				// ufoMeshData.Ns.push_back(128.0f);
				// ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
			}
			// --- Fins: pink ---
			else if (i >= static_cast<size_t>(ufoFinsStart) &&
					 i <  static_cast<size_t>(ufoFinsStart + ufoFinsCount))
			{	ufoMeshData.Ka.push_back({0.05f, 0.0f, 0.02f});   // small ambient
				ufoMeshData.Kd.push_back({1.00f, 0.00f, 0.80f});  // full-on pink
				ufoMeshData.Ks.push_back({0.9f, 0.6f, 0.9f}); 
				// ufoMeshData.Ka.push_back({0.30f, 0.05f, 0.15f});
				// ufoMeshData.Kd.push_back({1.00f, 0.20f, 0.80f});
				// ufoMeshData.Ks.push_back({0.90f, 0.50f, 0.90f});
				ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
				ufoMeshData.Ns.push_back(96.0f);
				ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
			}
			// --- Top cone + antenna: pink ---
			else if (i >= static_cast<size_t>(ufoTopStart) &&
					 i <  static_cast<size_t>(ufoTopStart + ufoTopCount))
			{
			ufoMeshData.Ka.push_back({0.05f, 0.0f, 0.02f});   // small ambient
				ufoMeshData.Kd.push_back({1.00f, 0.00f, 0.80f});  // full-on pink
				ufoMeshData.Ks.push_back({0.9f, 0.6f, 0.9f}); 
				// ufoMeshData.Ka.push_back({0.30f, 0.05f, 0.15f});
				// ufoMeshData.Kd.push_back({1.00f, 0.20f, 0.80f});
				// ufoMeshData.Ks.push_back({0.90f, 0.50f, 0.90f});
				ufoMeshData.Ke.push_back({0.0f,  0.0f,  0.0f});
				ufoMeshData.Ns.push_back(96.0f);
				ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
		}
		// --- Main body cylinder: glossy shiny black ---
		else
		{
			// Tiny ambient so the silhouette isn't completely lost
			ufoMeshData.Ka.push_back({0.9f, 0.9f, 0.9f});
		
			// Very dark diffuse so it still reads as black,
			// but can pick up a bit of coloured light
			ufoMeshData.Kd.push_back({1.0f, 1.0f, 1.0f});
		
			// Strong specular to get a clear glossy highlight
			ufoMeshData.Ks.push_back({1.0f, 1.0f, 1.0f});
		
			// No self-emission
			ufoMeshData.Ke.push_back({0.0f, 0.0f, 0.0f});
		
			// Lower shininess so the highlight is wider and more visible
			ufoMeshData.Ns.push_back(128.0f);
		
			// White vertex colour so the lighting terms do all the work
			ufoMeshData.colors.push_back({1.0f, 1.0f, 1.0f});
		}
	}
	
MeshGL ufoMesh;	// Create VAO with material properties (uses the same function as landing pads)
	GLuint ufoVAO = create_vao(ufoMeshData);
	ufoMesh.vao = ufoVAO;
	ufoMesh.vertexCount = static_cast<GLsizei>(ufoPositions.size());

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

    // Create UI shader
    ShaderProgram uiShader({
        { GL_VERTEX_SHADER,   "assets/cw2/ui.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/ui.frag" }
    });

    // Create UI renderer with the shader
    UIRenderer uiRenderer(1280, 720, uiShader);

    Button launchButton{"Launch", 0, 0, 120, 40, {0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    Button resetButton{"Reset", 0, 0, 120, 40, {0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

    // Point lights
    gPointLights[0].color = Vec3f{ 1.f,   0.f,   0.f   };
    gPointLights[1].color = Vec3f{ 0.f,   1.f,   0.f   };
    gPointLights[2].color = Vec3f{ 0.f,   0.7f,  1.f   };

    gPointLights[0].enabled = true;
    gPointLights[1].enabled = true;
    gPointLights[2].enabled = true;
    gDirectionalLightEnabled = true;

    // ---------------------
    // Particle system setup
    // ---------------------
    ShaderProgram particleProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/particle.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/particle.frag" }
    });

    // Allocate particle buffers
    glGenVertexArrays(1, &gParticleVAO);
    glGenBuffers(1,      &gParticleVBO);

    glBindVertexArray(gParticleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gParticleVBO);

    // reserve max particles (positions only)
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

    // Load particle texture (RGBA with alpha)
    gParticleTexture = load_texture_2d("assets/cw2/particlecopy.png", false);

    // Initialise all particles as dead
    for (int i = 0; i < kMaxParticles; ++i)
        gParticles[i].life = -1.0f;

    gAliveCount = 0;
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

        // ===== Time step =====
        static double lastTime = glfwGetTime();
        double currentTime     = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        if (gUfoAnim.active && !gUfoAnim.paused)
        {
            gUfoAnim.time += dt;
        }

        // ===== Projection =====
        float aspect     = fbwidth / fbheight;
        float fovRadians = 60.0f * std::numbers::pi_v<float> / 180.0f;
        float zNear      = 0.1f;
        float zFar       = 250.0f;

        Mat44f proj = make_perspective_projection(fovRadians, aspect, zNear, zFar);

        // ===== UFO animation =====

        Vec3f ufoStartPos{
            landingPadPos1.x,
            landingPadPos1.y + 1.25f,
            landingPadPos1.z
        };

        // Lights around body
        float lightRadius = 0.75f;
        float lightHeight = 0.6f;

        Vec3f lightOffset0{ -lightRadius * 0.5f, lightHeight, lightRadius * 0.866f };
        Vec3f lightOffset1{  lightRadius * 0.985f, lightHeight, -lightRadius * 0.174f };
        Vec3f lightOffset2{  0.f, lightHeight, -lightRadius };

        Vec3f ufoPos     = ufoStartPos;
        Vec3f forwardWS{ 0.f, 1.f, 0.f };
        Vec3f rightWS  { 1.f, 0.f, 0.f };
        Vec3f upWS     { 0.f, 0.f, 1.f };

        float u = 0.0f;

        if (gUfoAnim.active)
{
    float totalTime = 8.0f;
    float tAnim = gUfoAnim.time;
    if (tAnim < 0.f)       tAnim = 0.f;
    if (tAnim > totalTime) tAnim = totalTime;

    // Normalised time 0..1 along the curve
    u = tAnim / totalTime;

    // Shape parameters (similar overall scale to what you had)
    float rangeZ    = 80.0f;
    float maxHeight = 50.0f;

    float x0 = ufoStartPos.x;
    float y0 = ufoStartPos.y;
    float z0 = ufoStartPos.z;

    // --- Define Bézier control points in 3D ---
Vec3f A = ufoStartPos;

// B: mostly straight up from the pad (no Z offset)
//    => initial derivative is vertical, so it launches straight up
Vec3f B{
    x0,
    y0 + maxHeight * 0.7f,
    z0
};

// C: near the apex, partway downrange
Vec3f C{
    x0,
    y0 + maxHeight,
    z0 + rangeZ * 0.55f
};

// D: further downrange and lower
Vec3f D{
    x0,
    y0 + maxHeight * 0.2f,
    z0 + rangeZ
};


    // Position on the curve
    ufoPos = bezier3(A, B, C, D, u);

    // Tangent from the curve (finite difference on t)
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
        // Avoid degeneracy when flying straight up
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

        // Orientation: align rocket with the path direction (no blending)
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

// ---------------------------------
// Particle simulation (rocket exhaust)
// ---------------------------------

// Emit only when animation is active and not paused
if (gUfoAnim.active && !gUfoAnim.paused)
{
    // Engine position: very close to booster:
    // slightly behind the rocket and slightly "below" it.

    Vec3f enginePos = ufoPos - forwardWS  - upWS * 0.1f;

    // Vec3f enginePos = ufoPos
    //     - forwardWS * 2.f 
    //     - upWS      *  0.1f; // adjust as needed

    // Particles to spawn this frame
    gEmissionAccumulator += gEmissionRate * dt;
    int toSpawn = static_cast<int>(gEmissionAccumulator);
    if (toSpawn > 0)
        gEmissionAccumulator -= static_cast<float>(toSpawn);

    // Base velocity away from rocket
    Vec3f baseVel = -forwardWS * 7.0f;
// Spread parameters
    const float spreadRadius = 0.2f;   // how wide the plume is at the engine
    const float verticalSpread = 0.4f; // small variation in height

    for (int n = 0; n < toSpawn; ++n)
    {
        int idx = alloc_particle();
        if (idx < 0)
            break;

        // ---- Random offset in a disc around enginePos (wider spread) ----
        float u1 = std::rand() / float(RAND_MAX);
        float u2 = std::rand() / float(RAND_MAX);

        float r     = spreadRadius * std::sqrt(u1);                        // radius in [0, spreadRadius]
        float theta = 2.0f * std::numbers::pi_v<float> * u2;              // angle in [0, 2π)

        float dx = r * std::cos(theta);
        float dz = r * std::sin(theta);
        float dy = (std::rand() / float(RAND_MAX) - 0.5f) * verticalSpread;

        Vec3f offset = rightWS * dx + upWS * dz;

        // ---- Random jitter in velocity so trail is not a perfect line ----
        Vec3f jitter{
            (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f,
            (std::rand() / float(RAND_MAX) - 0.5f) * 3.0f,
            (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f
        };

        Vec3f vel = baseVel + jitter;

        // ---- Sub-frame positioning: avoid "grouped" bands each frame ----
        float tFrac = std::rand() / float(RAND_MAX); // 0..1
        Vec3f substepOffset = vel * (tFrac * dt);

        gParticles[idx].pos  = enginePos + offset;
        gParticles[idx].vel  = vel;
        gParticles[idx].life = 1.f;
    }
}


// Integrate motion – freeze when paused
if (!gUfoAnim.paused)
{
    for (int i = 0; i < kMaxParticles; ++i)
    {
        if (gParticles[i].life > 0.0f)
        {
            gParticles[i].life -= dt;
            if (gParticles[i].life > 0.0f)
            {
                gParticles[i].pos = gParticles[i].pos + gParticles[i].vel * dt;

                // Kill particles that go below the "sea level" plane y = 0
                if (gParticles[i].pos.y < -0.98f)
                {
                    gParticles[i].life = 0.0f;
                }
            }
        }
    }
}




        // ===== Camera movement (free) =====
        updateCameraMovement(gCamera, dt);

        std::print(
            "Camera position: ({:.2f}, {:.2f}, {:.2f})\n",
            gCamera.position.x, gCamera.position.y, gCamera.position.z
        );

        // Update button positions (bottom center)
        float buttonY = fbheight - 60.0f;  // 60px from bottom
        launchButton.x = fbwidth / 2.0f - 70.0f;  // Left button
        launchButton.y = buttonY;
        resetButton.x  = fbwidth / 2.0f + 70.0f;
        resetButton.y  = buttonY;

		// ----------------------------
// Upload alive particles to VBO
// ----------------------------

static Vec3f particlePositions[kMaxParticles];
int alive = 0;

for (int i = 0; i < kMaxParticles; ++i)
{
    if (gParticles[i].life > 0.0f)
    {
        particlePositions[alive++] = gParticles[i].pos;
    }
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
    // ===== SINGLE VIEW MODE =====
    glViewport(0, 0, static_cast<int>(fbwidth), static_cast<int>(fbheight));
    
    // Compute camera
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
        landingPadPos2
    );
}
else
{
    // ===== SPLIT SCREEN MODE =====
    
    // Left half
    int leftWidth = static_cast<int>(fbwidth) / 2;
    int rightWidth = static_cast<int>(fbwidth) - leftWidth;  // Handle odd widths
    int fullHeight = static_cast<int>(fbheight);
    
    glViewport(0, 0, leftWidth, fullHeight);
    
    // Compute projection for left view
    float aspectLeft = static_cast<float>(leftWidth) / static_cast<float>(fullHeight);
    Mat44f projLeft = make_perspective_projection(fovRadians, aspectLeft, zNear, zFar);
    
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
        landingPadPos2
    );
    
    // Right half
    glViewport(leftWidth, 0, rightWidth, fullHeight);
    
    // Compute projection for right view
    float aspectRight = static_cast<float>(rightWidth) / static_cast<float>(fullHeight);
    Mat44f projRight = make_perspective_projection(fovRadians, aspectRight, zNear, zFar);
    
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
        landingPadPos2
    );
    
    // Restore full viewport for next frame
    glViewport(0, 0, static_cast<int>(fbwidth), static_cast<int>(fbheight));
}

        // Render UI
        uiRenderer.setWindowSize(static_cast<int>(fbwidth), static_cast<int>(fbheight));
        uiRenderer.beginFrame();

        // Top-left altitude display
        char altitudeText[64];
        std::snprintf(altitudeText, sizeof(altitudeText), "Altitude: %.1f m", ufoPos.y);
        uiRenderer.renderText(10.0f, 10.0f, altitudeText, 24.0f, Vec4f{1.0f, 1.0f, 1.0f, 1.0f});

        // Bottom-center buttons
        if (uiRenderer.renderButton(launchButton, gMouseX, gMouseY, gMouseLeftDown))
        {
            // Launch clicked
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

        if (uiRenderer.renderButton(resetButton, gMouseX, gMouseY, gMouseLeftDown))
        {
            // Reset clicked
            gUfoAnim.active = false;
            gUfoAnim.paused = false;
            gUfoAnim.time = 0.f;

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
        {
            gCamera.moveForward = pressed || (aAction == GLFW_REPEAT);
        }
        else if (aKey == GLFW_KEY_S)
        {
            gCamera.moveBackward = pressed || (aAction == GLFW_REPEAT);
        }
        else if (aKey == GLFW_KEY_A)
        {
            gCamera.moveLeft = pressed || (aAction == GLFW_REPEAT);
        }
        else if (aKey == GLFW_KEY_D)
        {
            gCamera.moveRight = pressed || (aAction == GLFW_REPEAT);
        }
        else if (aKey == GLFW_KEY_E)
        {
            gCamera.moveUp = pressed || (aAction == GLFW_REPEAT);
        }
        else if (aKey == GLFW_KEY_Q)
        {
            gCamera.moveDown = pressed || (aAction == GLFW_REPEAT);
        }

        // Light toggles
        if (aAction == GLFW_PRESS)
        {
            if (aKey == GLFW_KEY_1)
            {
                gPointLights[0].enabled = !gPointLights[0].enabled;
            }
            else if (aKey == GLFW_KEY_2)
            {
                gPointLights[1].enabled = !gPointLights[1].enabled;
            }
            else if (aKey == GLFW_KEY_3)
            {
                gPointLights[2].enabled = !gPointLights[2].enabled;
            }
            else if (aKey == GLFW_KEY_4)
            {
                gDirectionalLightEnabled = !gDirectionalLightEnabled;
            }
        }

        // Speed modifiers
        if (aKey == GLFW_KEY_LEFT_SHIFT || aKey == GLFW_KEY_RIGHT_SHIFT)
        {
            gCamera.fast = pressed || (aAction == GLFW_REPEAT);
        }
        if (aKey == GLFW_KEY_LEFT_CONTROL || aKey == GLFW_KEY_RIGHT_CONTROL)
        {
            gCamera.slow = pressed || (aAction == GLFW_REPEAT);
        }

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

        // Toggle split screen with V key
        if (aKey == GLFW_KEY_V && aAction == GLFW_PRESS)
        {
            gSplitScreenEnabled = !gSplitScreenEnabled;
        }

        // Camera mode cycling - must check Shift FIRST
        if (aKey == GLFW_KEY_C && aAction == GLFW_PRESS)
        {
            bool shiftPressed = (glfwGetKey(aWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
                                glfwGetKey(aWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
            
            if (shiftPressed)
            {
                // Shift+C: Cycle camera mode 2 (right view)
                if (gCameraMode2 == CameraMode::Free)
                    gCameraMode2 = CameraMode::Chase;
                else if (gCameraMode2 == CameraMode::Chase)
                    gCameraMode2 = CameraMode::Ground;
                else
                    gCameraMode2 = CameraMode::Free;
            }
            else
            {
                // C only: Cycle camera mode 1 (left view)
                if (gCameraMode == CameraMode::Free)
                    gCameraMode = CameraMode::Chase;
                else if (gCameraMode == CameraMode::Chase)
                    gCameraMode = CameraMode::Ground;
                else
                    gCameraMode = CameraMode::Free;
            }
        }
    }

    void glfw_callback_mouse_button_( GLFWwindow* window, int button, int action, int )
    {
        // Handle left mouse button for UI
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            gMouseLeftDown = (action == GLFW_PRESS);
        }
        
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

        // Update mouse position for UI
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
