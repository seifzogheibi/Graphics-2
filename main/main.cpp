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

#include "defaults.hpp"

#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"



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

	Camera gCamera;

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
	//glDisable(GL_CULL_FACE);

	// Enable sRGB-correct framebuffer if you have sRGB textures / gamma-correct lighting
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Set a reasonable clear color (background)
	glClearColor(0.8f, 0.8f, 0.8f, 1.f);
	
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

// START
	struct MeshGL
{
    GLuint vao = 0;
    GLuint vboPositions = 0;
    GLuint vboNormals   = 0;
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

	//positions.reserve(100000);
	//normals.reserve(100000);

	// Iterate over all shapes and their indices
	for (auto const& shape : shapes)
	{
		auto const& mesh = shape.mesh;

		for (auto const& idx : mesh.indices)
		{
			// Position
			Vec3f pos{0.f, 0.f, 0.f};
			if (idx.position_index >= 0)
			{
				std::size_t pi = static_cast<std::size_t>(idx.position_index) * 3;
				pos[0] = attrib.positions[pi + 0];
				pos[1] = attrib.positions[pi + 1];
				pos[2] = attrib.positions[pi + 2];
			}
			positions.push_back(pos);

			// Normal
			Vec3f nrm{0.f, 1.f, 0.f}; // default up if no normal
			if (idx.normal_index >= 0)
			{
				std::size_t ni = static_cast<std::size_t>(idx.normal_index) * 3;
				nrm[0] = attrib.normals[ni + 0];
				nrm[1] = attrib.normals[ni + 1];
				nrm[2] = attrib.normals[ni + 2];
			}
			normals.push_back(nrm);
		}
	}

	// Store vertex count for drawing
	terrainMesh.vertexCount = static_cast<GLsizei>(positions.size());




	// === Create VBOs and VAO for terrain mesh ===

	glGenVertexArrays(1, &terrainMesh.vao);
	glBindVertexArray(terrainMesh.vao);

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

	// Unbind VAO & VBOs (good practice)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

ShaderProgram terrainProgram({
    { GL_VERTEX_SHADER,   "assets/cw2/default.vert" },
    { GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
});



	// === Static camera + transforms + light ===

// Model = identity (no extra transform on terrain)
Mat44f model = kIdentity44f;


// We’ll build view each frame, but we can store these

// Light & colors
Vec3f lightDir    = normalize(Vec3f{ 0.f, 1.f, -1.f });  // as spec says (0,1,-1)
Vec3f baseColor   = { 0.6f, 0.7f, 0.6f };               // terrain-ish colour
Vec3f ambientColor= { 0.1f, 0.1f, 0.1f };


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

// 0) Time step (seconds)
static double lastTime = glfwGetTime();
double currentTime = glfwGetTime();
float dt = static_cast<float>(currentTime - lastTime);
lastTime = currentTime;

// 1) Aspect ratio and projection matrix (depends on framebuffer size)
float aspect = fbwidth / fbheight;
float fovRadians = 60.0f * std::numbers::pi_v<float> / 180.0f;
float zNear = 0.1f;
float zFar  = 250.0f;

Mat44f proj = make_perspective_projection(fovRadians, aspect, zNear, zFar) * make_rotation_x(-gCamera.pitch) * make_rotation_y(gCamera.yaw) * make_translation(-gCamera.position);


// Base speed and modifiers
float baseSpeed = 5.0f;  // tweak this to taste (units per second)
if (gCamera.fast)
	baseSpeed *= 4.0f;      // Shift
if (gCamera.slow)
	baseSpeed *= 0.25f;     // Ctrl

float moveStep = baseSpeed * dt;

// Movement implementation
// Compute forward and right vectors from yaw and pitch
Vec3f forward{
	std::sin(gCamera.yaw) * std::cos(gCamera.pitch),
	std::sin(gCamera.pitch),
	-std::cos(gCamera.yaw) * std::cos(gCamera.pitch)
};
forward = normalize(forward);

Vec3f right{
	std::cos(gCamera.yaw),
	0.0f,
	std::sin(gCamera.yaw)
};
right = normalize(right);

Vec3f up{ 0.0f, 1.0f, 0.0f };

// Apply movement based on pressed keys
if (gCamera.moveForward)
	gCamera.position = gCamera.position + forward * moveStep;
if (gCamera.moveBackward)
	gCamera.position = gCamera.position - forward * moveStep;
if (gCamera.moveRight)
	gCamera.position = gCamera.position + right * moveStep;
if (gCamera.moveLeft)
	gCamera.position = gCamera.position - right * moveStep;
if (gCamera.moveUp)
	gCamera.position = gCamera.position + up * moveStep;
if (gCamera.moveDown)
	gCamera.position = gCamera.position - up * moveStep;

// 4) Build view matrix from camera position + orientation
Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model)) );

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame


// 1) Clear framebuffer
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 2) Use terrain shader program) 
GLuint progId = terrainProgram.programId();
glUseProgram(progId);

GLint locProj  = glGetUniformLocation(progId, "uProj");

glUniformMatrix4fv(locProj,  1, GL_TRUE, proj.v);
glUniformMatrix3fv(
	2, // make sure this matches the location = N in the vertex shader!
	1, GL_TRUE, normalMatrix.v
);

GLint locLightDir     = glGetUniformLocation(progId, "uLightDir");
GLint locBaseColor    = glGetUniformLocation(progId, "uBaseColor");
GLint locAmbientColor = glGetUniformLocation(progId, "uAmbientColor");

glUniform3fv(locLightDir,     1, &lightDir[0]);
glUniform3fv(locBaseColor,    1, &baseColor[0]);
glUniform3fv(locAmbientColor, 1, &ambientColor[0]);

// 4) Bind VAO and draw mesh
glBindVertexArray(terrainMesh.vao);
glDrawArrays(GL_TRIANGLES, 0, terrainMesh.vertexCount);
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
