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
#include "ufo.hpp" // added this basel

#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include <vector>



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

		struct Camera
	{
		Vec3f position{ 0.f, 0.f, 0.f };
		float yaw   = 0.0f;   // radians, left-right
		float pitch = 0.0f;   // radians, up-down

		bool moveForward = false;
		bool moveBackward = false;
		bool moveLeft = false;
		bool moveRight = false;
		bool moveUp = false;
		bool moveDown = false;

		bool fast = false;   // Shift
		bool slow = false;   // Ctrl

		bool mouseCaptured = false;
		bool firstMouse = true;
		double lastMouseX = 0.0;
		double lastMouseY = 0.0;
	};

	// ===== 1.8: camera modes =====
		enum class CameraMode
		{
			Free = 0,   // WASD + mouse
			Chase = 1,  // fixed distance behind/above rocket
			Ground = 2  // fixed point on ground, always looks at rocket
		};

		Camera gCamera;
		CameraMode gCameraMode = CameraMode::Free;

	// 1.7
	struct VehicleAnim
	{
		bool active = false;   // animation has been started at least once
		bool paused = false;   // toggled by F
		float time = 0.f;      // seconds since animation start
	};

	VehicleAnim gUfoAnim;

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


	// Forward declarations for extra callbacks
	void glfw_callback_mouse_button_( GLFWwindow* window, int button, int action, int mods );
	void glfw_callback_cursor_pos_( GLFWwindow* window, double xpos, double ypos );

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

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

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
	// TODO: Additional event handling setup

	glfwSetMouseButtonCallback( window, &glfw_callback_mouse_button_ );
	glfwSetCursorPosCallback( window, &glfw_callback_cursor_pos_ );
	glfwSetKeyCallback( window, &glfw_callback_key_ );

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoadGLLoader() failed - cannot load GL API!" );

	std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
	std::print( "VENDOR {}\n", (char const*)glGetString( GL_VENDOR ) );
	std::print( "VERSION {}\n", (char const*)glGetString( GL_VERSION ) );
	std::print( "SHADING_LANGUAGE_VERSION {}\n", (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here
	// start

	// Enable depth testing so nearer fragments occlude farther ones
	glEnable(GL_DEPTH_TEST);

	// Enable back-face culling (triangles facing away from camera are discarded)
	glEnable(GL_CULL_FACE);

	// Enable sRGB-correct framebuffer if you have sRGB textures / gamma-correct lighting
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Set a reasonable clear color (background)
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	
	// END


	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: global GL setup goes here
	// Basel/Seif code edit
    // START

	struct MeshGL
{
    GLuint vao = 0;
    GLuint vboPositions = 0;
    GLuint vboNormals   = 0;
    GLsizei vertexCount = 0;
	GLuint vboTexcoords = 0;
};

struct MeshGLColored
	{
		GLuint vao = 0;
		GLuint vboPositions = 0;
		GLuint vboNormals   = 0;
		GLuint vboColors    = 0;  // NEW: per-vertex colours
		GLsizei vertexCount = 0;
	};

	// === Load Parlahti mesh with rapidobj ===
	MeshGL terrainMesh;

    // 1) Parse the OBJ file
    rapidobj::Result objResult = rapidobj::ParseFile("assets/cw2/parlahti.obj");
    
	// Check for errors
    if (objResult.error)
    {
        throw Error("Fail", objResult.error.code.message());
    }

    // Ensure we have triangles
    rapidobj::Triangulate(objResult);

    // Shortcuts
    auto const& attrib = objResult.attributes;
    auto const& shapes = objResult.shapes;


	// 2) Flatten mesh into per-vertex arrays (no indexing, simpler)
	std::vector<Vec3f> positions;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> texcoords;

	for (auto const& shape : shapes)
	{
		auto const& mesh = shape.mesh;

		for (auto const& idx : mesh.indices)
		{
			// --- Position ---
			Vec3f pos{0.f, 0.f, 0.f};
			if (idx.position_index >= 0)
			{
				std::size_t pi = static_cast<std::size_t>(idx.position_index) * 3;
				pos[0] = attrib.positions[pi + 0];
				pos[1] = attrib.positions[pi + 1];
				pos[2] = attrib.positions[pi + 2];
			}
			positions.push_back(pos);

			// --- Normal ---
			Vec3f nrm{0.f, 1.f, 0.f}; // default up if no normal
			if (idx.normal_index >= 0)
			{
				std::size_t ni = static_cast<std::size_t>(idx.normal_index) * 3;
				nrm[0] = attrib.normals[ni + 0];
				nrm[1] = attrib.normals[ni + 1];
				nrm[2] = attrib.normals[ni + 2];
			}
			normals.push_back(nrm);

			// --- Texcoord (UV) ---
			Vec2f uv{0.f, 0.f};
			if (idx.texcoord_index >= 0)
			{
				std::size_t ti = static_cast<std::size_t>(idx.texcoord_index) * 2;
				uv = Vec2f{
					attrib.texcoords[ti + 0],
					attrib.texcoords[ti + 1]
				};
			}
			texcoords.push_back(uv);
		}
	}

	// === Create VAO for terrain ===
	glGenVertexArrays(1, &terrainMesh.vao);
	glBindVertexArray(terrainMesh.vao);


    // Store vertex count for drawing
    terrainMesh.vertexCount = static_cast<GLsizei>(positions.size());


    // --- Positions VBO (location = 0) ---
    glGenBuffers(1, &terrainMesh.vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, terrainMesh.vboPositions);
    glBufferData(
        GL_ARRAY_BUFFER,
        positions.size() * sizeof(Vec3f),
        positions.data(),
        GL_STATIC_DRAW
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // layout(location = 0)
        3,                  // x, y, z
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vec3f),
        (void*)0
    );

    // --- Normals VBO (location = 1) ---
    glGenBuffers(1, &terrainMesh.vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, terrainMesh.vboNormals);
    glBufferData(
        GL_ARRAY_BUFFER,
        normals.size() * sizeof(Vec3f),
        normals.data(),
        GL_STATIC_DRAW
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                  // layout(location = 1)
        3,                  // nx, ny, nz
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vec3f),
        (void*)0
    );


    ShaderProgram terrainProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/default.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
    });

    // === Static transforms + light ===

    // Model = identity (no extra transform on terrain)
    Mat44f model = kIdentity44f;

    // Light & colors
    Vec3f lightDir     = normalize(Vec3f{ 0.f, 1.f, -1.f });  // (0,1,-1)
    Vec3f baseColor    = { 0.6f, 0.7f, 0.6f };                // terrain-ish colour
    Vec3f ambientColor = { 0.1f, 0.1f, 0.1f };

    // === UFO CPU build (from your Task1.5) – we’ll flesh this out next ===
	MeshGL ufoMesh;

	// Build UFO geometry on CPU
	std::vector<Vec3f> ufoPositions;
	std::vector<Vec3f> ufoNormals;

	// counts for base (grey) and top+antenna (blue)
	int ufoBaseVertexCount = 0;
	int ufoTopVertexCount  = 0;

	buildUfoFlatArrays(
		ufoPositions,
		ufoNormals,
		ufoBaseVertexCount,
		ufoTopVertexCount
	);





	// --- Texcoords VBO (location = 3) ---
	glGenBuffers(1, &terrainMesh.vboTexcoords);
	glBindBuffer(GL_ARRAY_BUFFER, terrainMesh.vboTexcoords);
	glBufferData(
		GL_ARRAY_BUFFER,
		texcoords.size() * sizeof(Vec2f),
		texcoords.data(),
		GL_STATIC_DRAW
	);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,                  // layout(location = 3)
		2,                  // u, v
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2f),
		(void*)0
	);


	// Unbind VAO & VBOs (good practice)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Create VAO/VBO for UFO (same pattern as terrain)
    glGenVertexArrays(1, &ufoMesh.vao);
    glBindVertexArray(ufoMesh.vao);

    glGenBuffers(1, &ufoMesh.vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, ufoMesh.vboPositions);
    glBufferData(
        GL_ARRAY_BUFFER,
        ufoPositions.size() * sizeof(Vec3f),
        ufoPositions.data(),
        GL_STATIC_DRAW
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vec3f), (void*)0
    );


    glGenBuffers(1, &ufoMesh.vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, ufoMesh.vboNormals);
    glBufferData(
        GL_ARRAY_BUFFER,
        ufoNormals.size() * sizeof(Vec3f),
        ufoNormals.data(),
        GL_STATIC_DRAW
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vec3f), (void*)0
    );

    // store count
    ufoMesh.vertexCount = static_cast<GLsizei>(ufoPositions.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	GLuint terrainTexture = load_texture_2d("assets/cw2/L4343A-4k.jpeg");

ShaderProgram landingProgram({
    { GL_VERTEX_SHADER,   "assets/cw2/landing.vert" },
    { GL_FRAGMENT_SHADER, "assets/cw2/landing.frag" }
});

	// === Static camera + transforms + light ===

// We’ll build view each frame, but we can store these

// Landing pad positions (you will tweak these by flying around to put them in the sea)
Vec3f landingPadPos1{ 1.f, -.7f, -20.f };
Vec3f landingPadPos2{ 8.f, -.7f, 40.f }; 



	// === Load Landing Pad mesh (coloured, per material) ===

	MeshGLColored landingMesh;

	rapidobj::Result lpResult = rapidobj::ParseFile("assets/cw2/landingpad.obj");
	if (lpResult.error)
	{
		throw Error("Failed to load landingpad.obj: {}", lpResult.error.code.message());
	}

	rapidobj::Triangulate(lpResult);

	auto const& lpAttrib    = lpResult.attributes;
	auto const& lpShapes    = lpResult.shapes;
	auto const& lpMaterials = lpResult.materials;

	std::vector<Vec3f> lpPositions;
	std::vector<Vec3f> lpNormals;
	std::vector<Vec3f> lpColors;

	for (auto const& shape : lpShapes)
	{
		auto const& mesh = shape.mesh;

		// Each 3 indices (after Triangulate) correspond to 1 triangle = 1 face
		for (std::size_t idx_i = 0; idx_i < mesh.indices.size(); ++idx_i)
		{
			auto const& idx = mesh.indices[idx_i];

			// --- Position ---
			Vec3f pos{ 0.f, 0.f, 0.f };
			if (idx.position_index >= 0)
			{
				std::size_t pi = static_cast<std::size_t>(idx.position_index) * 3;
				pos = Vec3f{
					lpAttrib.positions[pi + 0],
					lpAttrib.positions[pi + 1],
					lpAttrib.positions[pi + 2]
				};
			}
			lpPositions.push_back(pos);

			// --- Normal ---
			Vec3f nrm{ 0.f, 1.f, 0.f };
			if (idx.normal_index >= 0)
			{
				std::size_t ni = static_cast<std::size_t>(idx.normal_index) * 3;
				nrm = Vec3f{
					lpAttrib.normals[ni + 0],
					lpAttrib.normals[ni + 1],
					lpAttrib.normals[ni + 2]
				};
			}
			lpNormals.push_back(nrm);

			// --- Colour from material diffuse (Kd) ---
			// Figure out which face we are on and thus which material
			std::size_t faceIndex = idx_i / 3; // 3 indices per triangle
			int matId = -1;
			if (!mesh.material_ids.empty() && faceIndex < mesh.material_ids.size())
				matId = mesh.material_ids[faceIndex];

			Vec3f col{ 1.f, 1.f, 1.f }; // default white if something's off
			if (matId >= 0 && static_cast<std::size_t>(matId) < lpMaterials.size())
			{
				auto const& mat = lpMaterials[matId];
				col = Vec3f{
					mat.diffuse[0],
					mat.diffuse[1],
					mat.diffuse[2]
				};
			}
			lpColors.push_back(col);
		}
	}

	landingMesh.vertexCount = static_cast<GLsizei>(lpPositions.size());

	
	
	// === Create VAO + VBOs for landing pad ===

	glGenVertexArrays(1, &landingMesh.vao);
	glBindVertexArray(landingMesh.vao);

	// Positions at location = 0
	glGenBuffers(1, &landingMesh.vboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, landingMesh.vboPositions);
	glBufferData(
		GL_ARRAY_BUFFER,
		lpPositions.size() * sizeof(Vec3f),
		lpPositions.data(),
		GL_STATIC_DRAW
	);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3f),
		(void*)0
	);

	// Normals at location = 1
	glGenBuffers(1, &landingMesh.vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, landingMesh.vboNormals);
	glBufferData(
		GL_ARRAY_BUFFER,
		lpNormals.size() * sizeof(Vec3f),
		lpNormals.data(),
		GL_STATIC_DRAW
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3f),
		(void*)0
	);

	// Colours at location = 3  (same location index you used for terrain texcoords,
	// but in a DIFFERENT VAO, so that's fine)
	glGenBuffers(1, &landingMesh.vboColors);
	glBindBuffer(GL_ARRAY_BUFFER, landingMesh.vboColors);
	glBufferData(
		GL_ARRAY_BUFFER,
		lpColors.size() * sizeof(Vec3f),
		lpColors.data(),
		GL_STATIC_DRAW
	);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,
		3,          // r, g, b
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3f),
		(void*)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	// END
    OGL_CHECKPOINT_ALWAYS();


	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state
		//TODO: update state

		// Update state

// Update state

        // ========================
        // Update state (time, UFO, camera)
        // ========================

        // 0) Time step (seconds)
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Update UFO animation time (task 1.7)
        if (gUfoAnim.active && !gUfoAnim.paused)
        {
            gUfoAnim.time += dt;
        }

        // 1) Aspect ratio and projection matrix (depends on framebuffer size)
        float aspect     = fbwidth / fbheight;
        float fovRadians = 60.0f * std::numbers::pi_v<float> / 180.0f;
        float zNear      = 0.1f;
        float zFar       = 250.0f;

        // Pure projection matrix (no view yet)
        Mat44f projMat = make_perspective_projection(fovRadians, aspect, zNear, zFar);

        // ========================
        // 2) UFO animation (position + orientation) – Task 1.7
        // ========================

        // Start position (above landing pad A)
        Vec3f ufoStartPos{
            landingPadPos1.x,
            landingPadPos1.y + 0.5f,
            landingPadPos1.z
        };

        // Defaults (parked, pointing up)
        Vec3f ufoPos     = ufoStartPos;
        Vec3f forwardWS{ 0.f, 1.f, 0.f };   // rocket nose
        Vec3f rightWS  { 1.f, 0.f, 0.f };
        Vec3f upWS     { 0.f, 0.f, 1.f };

        if (gUfoAnim.active)
        {
            float totalTime = 8.0f;  // seconds to reach "space" (tweak)
            float tAnim = gUfoAnim.time;
            if (tAnim < 0.f) tAnim = 0.f;

            float u = tAnim / totalTime;   // 0..1
            if (u > 1.f) u = 1.f;

            // Ease-in: start slow, then accelerate
            float s = u * u;

            // Bézier control points
            Vec3f P0 = ufoStartPos;
            Vec3f P1 = ufoStartPos + Vec3f{ 0.f,   3.f,   0.f };
            Vec3f P2 = ufoStartPos + Vec3f{ 0.f,  15.f, -20.f };
            Vec3f P3 = ufoStartPos + Vec3f{ 0.f,  40.f, -80.f };

            // Current position
            ufoPos = bezier3(P0, P1, P2, P3, s);

            // Approximate direction of motion
            float eps = 0.01f;
            float s2  = s + eps;
            if (s2 > 1.f) s2 = 1.f;

            Vec3f ufoPosAhead = bezier3(P0, P1, P2, P3, s2);
            Vec3f vel = ufoPosAhead - ufoPos;
            float speed = length(vel);

            if (speed > 1e-4f)
            {
                forwardWS = vel / speed;       // direction of movement

                Vec3f worldUp{ 0.f, 1.f, 0.f };
                if (std::fabs(dot(forwardWS, worldUp)) > 0.99f)
                {
                    worldUp = Vec3f{ 1.f, 0.f, 0.f };
                }

                // Right-handed basis
                rightWS = normalize(cross(worldUp, forwardWS));
                upWS    = cross(forwardWS, rightWS);
            }
        }
        else
        {
            // Not active: parked
            ufoPos    = ufoStartPos;
            forwardWS = Vec3f{ 0.f, 1.f, 0.f };
            rightWS   = Vec3f{ 1.f, 0.f, 0.f };
            upWS      = Vec3f{ 0.f, 0.f, 1.f };
        }

        // Build rotation matrix from basis vectors:
        // local X -> rightWS, local Y -> forwardWS, local Z -> upWS
        Mat44f ufoRot = kIdentity44f;

        // Column 0 = right
        ufoRot[0,0] = rightWS.x;
        ufoRot[1,0] = rightWS.y;
        ufoRot[2,0] = rightWS.z;

        // Column 1 = forward (nose)
        ufoRot[0,1] = forwardWS.x;
        ufoRot[1,1] = forwardWS.y;
        ufoRot[2,1] = forwardWS.z;

        // Column 2 = up
        ufoRot[0,2] = upWS.x;
        ufoRot[1,2] = upWS.y;
        ufoRot[2,2] = upWS.z;

        // Full UFO model (position + orientation + scale)
        Mat44f ufoModel =
            make_translation(ufoPos) *
            ufoRot *
            make_scaling(0.5f, 0.5f, 0.5f);

        // ========================
        // 3) Camera movement (free mode) – same as before
        // ========================

        // Base speed and modifiers
        float baseSpeed = 5.0f;  // tweak to taste
        if (gCamera.fast)
            baseSpeed *= 4.0f;      // Shift
        if (gCamera.slow)
            baseSpeed *= 0.25f;     // Ctrl

        float moveStep = baseSpeed * dt;

        // Compute forward and right vectors from yaw and pitch
        Vec3f camForward{
            std::sin(gCamera.yaw) * std::cos(gCamera.pitch),
            std::sin(gCamera.pitch),
            -std::cos(gCamera.yaw) * std::cos(gCamera.pitch)
        };
        camForward = normalize(camForward);

        Vec3f camRight{
            std::cos(gCamera.yaw),
            0.0f,
            std::sin(gCamera.yaw)
        };
        camRight = normalize(camRight);

        Vec3f camUp{ 0.0f, 1.0f, 0.0f };

        // Only really meaningful in Free mode, but we always update gCamera
        if (gCamera.moveForward)
            gCamera.position = gCamera.position + camForward * moveStep;
        if (gCamera.moveBackward)
            gCamera.position = gCamera.position - camForward * moveStep;
        if (gCamera.moveRight)
            gCamera.position = gCamera.position + camRight * moveStep;
        if (gCamera.moveLeft)
            gCamera.position = gCamera.position - camRight * moveStep;
        if (gCamera.moveUp)
            gCamera.position = gCamera.position + camUp * moveStep;
        if (gCamera.moveDown)
            gCamera.position = gCamera.position - camUp * moveStep;

        std::print(
            "Camera position: ({:.2f}, {:.2f}, {:.2f})\n",
            gCamera.position.x, gCamera.position.y, gCamera.position.z
        );

        // ========================
        // 4) Build VIEW matrix based on camera mode – Task 1.8
        // ========================

        Mat44f view;

        switch (gCameraMode)
        {
        case CameraMode::Free:
        {
            // Your original free camera
            view =
                make_rotation_x(-gCamera.pitch) *
                make_rotation_y(gCamera.yaw) *
                make_translation(-gCamera.position);
            break;
        }
        case CameraMode::Chase:
        {
            // Camera sits behind and above the rocket
            float distBack = 8.0f;   // distance behind
            float heightUp = 3.0f;   // height above

            Vec3f camPos    = ufoPos - forwardWS * distBack + upWS * heightUp;
            Vec3f camTarget = ufoPos + forwardWS * 1.0f; // look slightly ahead
            Vec3f dir       = normalize(camTarget - camPos);

            float camPitch = std::asin(dir.y);
            float camYaw   = std::atan2(dir.x, -dir.z);

            view =
                make_rotation_x(-camPitch) *
                make_rotation_y(camYaw) *
                make_translation(-camPos);
            break;
        }
        case CameraMode::Ground:
        {
            // Fixed point on ground that always looks at the rocket
            Vec3f camPos{
                landingPadPos1.x + 25.f,
                landingPadPos1.y + 5.f,
                landingPadPos1.z + 25.f
            };

            Vec3f camTarget = ufoPos;
            Vec3f dir       = normalize(camTarget - camPos);

            float camPitch = std::asin(dir.y);
            float camYaw   = std::atan2(dir.x, -dir.z);

            view =
                make_rotation_x(-camPitch) *
                make_rotation_y(camYaw) *
                make_translation(-camPos);
            break;
        }
        }

        // ========================
        // 5) Combine projection + view
        // ========================
        Mat44f viewProj = projMat * view;

        // Terrain model is identity, so MVP_terrain = viewProj * model
        Mat44f terrainMvp = viewProj * model;

        // UFO MVP
        Mat44f ufoMvp = viewProj * ufoModel;

        // 6) Normal matrix (still fine with model = identity)
        Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model)) );

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame


// 1) Clear framebuffer
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 2) Use shader program
GLuint progId = terrainProgram.programId();
glUseProgram(progId);

// Look up uniforms once per frame
GLint locProj         = glGetUniformLocation(progId, "uProj");

// lighting
GLint locLightDir     = glGetUniformLocation(progId, "uLightDir");
GLint locBaseColor    = glGetUniformLocation(progId, "uBaseColor");
GLint locAmbientColor = glGetUniformLocation(progId, "uAmbientColor");

// Lighting uniforms (same for both objects)
glUniform3fv(locLightDir,     1, &lightDir[0]);
glUniform3fv(locAmbientColor, 1, &ambientColor[0]);

// Normal matrix (still fine for both; only uniform scaling)
glUniformMatrix3fv(
    2, // matches layout(location = 2) uniform mat3 uNormalMatrix;
    1, GL_TRUE, normalMatrix.v
);

// ====================
// Draw TERRAIN
// ====================
glUniformMatrix4fv(locProj, 1, GL_TRUE, terrainMvp.v);

// Terrain colour from your 1.3 work
glUniform3fv(locBaseColor, 1, &baseColor[0]);


// texture
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, terrainTexture);
GLint locTexture = glGetUniformLocation(progId, "uTexture");
glUniform1i(locTexture, 0);

// draw terrain

glBindVertexArray(terrainMesh.vao);
glDrawArrays(GL_TRIANGLES, 0, terrainMesh.vertexCount);
glBindVertexArray(0);

// ====================
// Draw UFO
// ====================

// Colours
Vec3f saucerColor{ 0.35f, 0.35f, 0.38f };  // dark-ish grey base
Vec3f domeColor  { 0.3f,  0.55f, 0.95f };  // light blue dome+antenna

glBindVertexArray(ufoMesh.vao);

// 1) Base saucer (grey) : vertices [0, ufoBaseVertexCount)
glUniform3fv(locBaseColor, 1, &saucerColor[0]);
glUniformMatrix4fv(locProj, 1, GL_TRUE, ufoMvp.v);
glDrawArrays(GL_TRIANGLES, 0, ufoBaseVertexCount);

// 2) Top dome + antenna (light blue) : vertices [ufoBaseVertexCount, ufoBaseVertexCount + ufoTopVertexCount)
glUniform3fv(locBaseColor, 1, &domeColor[0]);
glDrawArrays(GL_TRIANGLES, ufoBaseVertexCount, ufoTopVertexCount);

glBindVertexArray(0);


// === Draw landing pad twice (no duplicated data) ===

		GLuint landingProgId = landingProgram.programId();
		glUseProgram(landingProgId);

		// Normal matrix (location = 2 in the vertex shader)
		glUniformMatrix3fv(
			2,
			1, GL_TRUE, normalMatrix.v
		);

		// Lighting uniforms
		GLint lpLocLightDir     = glGetUniformLocation(landingProgId, "uLightDir");
		GLint lpLocAmbientColor = glGetUniformLocation(landingProgId, "uAmbientColor");
		glUniform3fv(lpLocLightDir,     1, &lightDir[0]);
		glUniform3fv(lpLocAmbientColor, 1, &ambientColor[0]);

		// MVP uniform location
		GLint lpLocProj = glGetUniformLocation(landingProgId, "uProj");

		glBindVertexArray(landingMesh.vao);

		// First landing pad
		Mat44f lpModel1 = make_translation(landingPadPos1);
		Mat44f lpMvp1   = viewProj * lpModel1;  // proj already includes camera transform
		glUniformMatrix4fv(lpLocProj, 1, GL_TRUE, lpMvp1.v);
		glDrawArrays(GL_TRIANGLES, 0, landingMesh.vertexCount);

		// Second landing pad
		Mat44f lpModel2 = make_translation(landingPadPos2);
		Mat44f lpMvp2   = viewProj * lpModel2;
		glUniformMatrix4fv(lpLocProj, 1, GL_TRUE, lpMvp2.v);
		glDrawArrays(GL_TRIANGLES, 0, landingMesh.vertexCount);

		glBindVertexArray(0);


		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	//TODO: additional cleanup
	
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
		// if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		// {
		// 	glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
		// 	return;
		// }

	bool pressed  = (aAction == GLFW_PRESS);
	// bool released = (aAction == GLFW_RELEASE);

	if (aKey == GLFW_KEY_ESCAPE && pressed)
	{
		glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
		return;
	}

	// Movement keys
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

	// Speed modifiers
	if (aKey == GLFW_KEY_LEFT_SHIFT || aKey == GLFW_KEY_RIGHT_SHIFT)
	{
		gCamera.fast = pressed || (aAction == GLFW_REPEAT);
	}
	if (aKey == GLFW_KEY_LEFT_CONTROL || aKey == GLFW_KEY_RIGHT_CONTROL)
	{
		gCamera.slow = pressed || (aAction == GLFW_REPEAT);
	}


		    // --- Task 1.7: UFO animation controls ---
    if (aKey == GLFW_KEY_F && aAction == GLFW_PRESS)
    {
        if (!gUfoAnim.active)
        {
            // Start animation from the beginning
            gUfoAnim.active = true;
            gUfoAnim.paused = false;
            gUfoAnim.time   = 0.f;
        }
        else
        {
            // Toggle pause/unpause
            gUfoAnim.paused = !gUfoAnim.paused;
        }
    }

    if (aKey == GLFW_KEY_R && aAction == GLFW_PRESS)
    {
        // Reset to original position and fully stop animation
        gUfoAnim.active = false;
        gUfoAnim.paused = false;
        gUfoAnim.time   = 0.f;
    }


	    // --- Task 1.8: cycle camera mode with C ---
    if (aKey == GLFW_KEY_C && aAction == GLFW_PRESS)
    {
        if (gCameraMode == CameraMode::Free)
        {
            gCameraMode = CameraMode::Chase;
        }
        else if (gCameraMode == CameraMode::Chase)
        {
            gCameraMode = CameraMode::Ground;
        }
        else
        {
            gCameraMode = CameraMode::Free;
        }
    }

	}


	void glfw_callback_mouse_button_( GLFWwindow* window, int button, int action, int )
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		gCamera.mouseCaptured = !gCamera.mouseCaptured;

		if (gCamera.mouseCaptured)
		{
			// Capture mouse
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			gCamera.firstMouse = true; // so we don't jump on first move
		}
		else
		{
			// Release mouse
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void glfw_callback_cursor_pos_( GLFWwindow*, double xpos, double ypos )
{
	if (!gCamera.mouseCaptured)
		return;

	if (gCamera.firstMouse)
	{
		gCamera.lastMouseX = xpos;
		gCamera.lastMouseY = ypos;
		gCamera.firstMouse = false;
		return;
	}

	double xoffset = xpos - gCamera.lastMouseX;
	double yoffset = ypos - gCamera.lastMouseY;
	gCamera.lastMouseX = xpos;
	gCamera.lastMouseY = ypos;

	// Sensitivity (tweak to your liking)
	float sensitivity = 0.0025f; // radians per pixel-ish
	gCamera.yaw   += static_cast<float>(xoffset) * sensitivity;
	gCamera.pitch -= static_cast<float>(yoffset) * sensitivity; // minus so moving mouse up looks up

	// Clamp pitch to avoid flipping
	float maxPitch = 1.5f; // about 86 degrees
	if (gCamera.pitch > maxPitch)  gCamera.pitch = maxPitch;
	if (gCamera.pitch < -maxPitch) gCamera.pitch = -maxPitch;
}
}

namespace
{
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
