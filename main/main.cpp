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
#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include <vector>

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

	struct Camera
{
	Vec3f position{ 0.f, 5.f, 0.f };  // Start above the terrain
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

	
	struct PointLight
	{
		Vec3f position;
		Vec3f color;
		bool enabled = true;
	};

	PointLight gPointLights[3];
    bool gDirectionalLightEnabled = true;

	// Forward declarations for extra callbacks
	void glfw_callback_mouse_button_( GLFWwindow* , int , int , int); // remove vars?
	void glfw_callback_cursor_pos_( GLFWwindow* , double, double); //remove vars?

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

	SimpleMeshData terrainMeshData = load_wavefront_obj("assets/cw2/parlahti.obj");
	GLuint terrainVAO = create_vao(terrainMeshData);



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
    Vec3f ambientColor = { 0.18f, 0.18f, 0.18f };
	Vec3f diffuseColor = { 0.6f, 0.6f, 0.6f };


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

	// Create SimpleMeshData with material properties for proper Blinn-Phong lighting
	SimpleMeshData ufoMeshData;
	ufoMeshData.positions = ufoPositions;
	ufoMeshData.normals = ufoNormals;
	
	// Add material properties for each vertex
	for (size_t i = 0; i < ufoPositions.size(); ++i)
	{
		if (i < static_cast<size_t>(ufoBaseVertexCount))
		{
			// Base (grey metallic material)
			ufoMeshData.colors.push_back(Vec3f{0.3f, 0.3f, 0.35f});
			ufoMeshData.Ka.push_back(Vec3f{0.2f, 0.2f, 0.25f});   // ambient
			ufoMeshData.Kd.push_back(Vec3f{0.4f, 0.4f, 0.45f});   // diffuse
			ufoMeshData.Ks.push_back(Vec3f{0.8f, 0.8f, 0.9f});    // specular (shiny metal)
			ufoMeshData.Ke.push_back(Vec3f{0.0f, 0.0f, 0.0f});    // no emission
			ufoMeshData.Ns.push_back(64.0f);                // shiny
		}
		else
		{
			// Top/dome (blue glass-like material)
			ufoMeshData.colors.push_back(Vec3f{0.2f, 0.4f, 0.8f});
			ufoMeshData.Ka.push_back(Vec3f{0.1f, 0.2f, 0.4f});    // ambient
			ufoMeshData.Kd.push_back(Vec3f{0.3f, 0.5f, 0.9f});    // diffuse
			ufoMeshData.Ks.push_back(Vec3f{0.9f, 0.9f, 1.0f});    // specular (glass-like)
			ufoMeshData.Ke.push_back(Vec3f{0.0f, 0.0f, 0.0f});    // no emission
			ufoMeshData.Ns.push_back(128.0f);               // very shiny
		}
	}
	MeshGL ufoMesh;
	
	// Create VAO with material properties (uses the same function as landing pads)
	GLuint ufoVAO = create_vao(ufoMeshData);
	ufoMesh.vao = ufoVAO;
	ufoMesh.vertexCount = static_cast<GLsizei>(ufoPositions.size());

	GLuint terrainTexture = load_texture_2d( (ASSETS + terrainMeshData.texture_filepath).c_str() );

ShaderProgram landingProgram({
    { GL_VERTEX_SHADER,   "assets/cw2/landing.vert" },
    { GL_FRAGMENT_SHADER, "assets/cw2/landing.frag" }
});

// === Static camera + transforms + light ===

// We’ll build view each frame, but we can store these

// Landing pad positions (you will tweak these by flying around to put them in the sea)
Vec3f landingPadPos1{ -11.50, -0.96, -54 };
Vec3f landingPadPos2{ 8.f, -.96f, 40.f };



	// === Load Landing Pad mesh (coloured, per material) ===

	SimpleMeshData landingMeshData = load_wavefront_obj("assets/cw2/landingpad.obj");
	GLuint landingVao = create_vao(landingMeshData);

	// === Setup point lights (world space) ===
	// These are approximate positions near your space vehicle – you can tweak later.

	// gPointLights[0].position = Vec3f{  0.f, 5.f,   0.f };  // e.g. top centre
	gPointLights[0].color    = Vec3f{  1.f,  0.f,   0.f };  // red

	// gPointLights[1].position = Vec3f{  5.f, 5.f,  -3.f };  // side
	gPointLights[1].color    = Vec3f{  0.f,  1.f,   0.f };  // green

	// gPointLights[2].position = Vec3f{ -4.f, 5.f,   2.f };  // other side
	gPointLights[2].color    = Vec3f{  0.f,  0.7f,  1.f };  // blue-ish

	gPointLights[0].enabled = true;
	gPointLights[1].enabled = true;
	gPointLights[2].enabled = true;

	gDirectionalLightEnabled = true;



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


		// Pure projection: no camera transforms here
		Mat44f proj = make_perspective_projection(fovRadians, aspect, zNear, zFar);


        // ========================
        // 2) UFO animation (position + orientation) – Task 1.7
        // ========================

        // Start position (above landing pad A)
        Vec3f ufoStartPos{
            landingPadPos1.x,
            landingPadPos1.y + 1.25f,
            landingPadPos1.z
        };

// === Attach point lights to the rocket ===
// Position lights around the cylindrical body (radius ~0.30 after scaling)
// Distributed at 120° intervals around the cylinder

float lightRadius = 0.75f;  // slightly outside body surface
float lightHeight = 0.6f;  // middle of rocket body

// Lights positioned around the body
Vec3f lightOffset0{ -lightRadius * 0.5f, lightHeight, lightRadius * 0.866f };    // Red: 145°
Vec3f lightOffset1{ lightRadius * 0.985f, lightHeight, -lightRadius * 0.174f };  // Green: 260°
Vec3f lightOffset2{ 0.f, lightHeight, -lightRadius};                             // Blue: 270° (0° - 90° = 270°)

// === End attach point lights ===

        // Defaults (parked, pointing up)
        Vec3f ufoPos     = ufoStartPos;
        Vec3f forwardWS{ 0.f, 1.f, 0.f };   // rocket nose
        Vec3f rightWS  { 1.f, 0.f, 0.f };
        Vec3f upWS     { 0.f, 0.f, 1.f };

        // Declare animation variables outside the block so they're accessible later
         // Declare animation parameter (0..1) – used for rotation too
        float u = 0.0f;

        if (gUfoAnim.active)
        {
            float totalTime = 8.0f;
            float tAnim = gUfoAnim.time;
            if (tAnim < 0.f)         tAnim = 0.f;
            if (tAnim > totalTime)   tAnim = totalTime;

            // Normalised time
            u = tAnim / totalTime;

            // Ease-in: start slow, then accelerate
            float s = u * u;   // 0..1

            // --- Smooth “parabolic” like path with vertical tangent at start ---

            // World up
            Vec3f worldUp{ 0.f, 1.f, 0.f };

            // Shape controls
            float rangeZ    = 80.0f;  // how far in -Z to travel
            float maxHeight = 50.0f;  // how high the arc goes

            // Start position
            float x0 = ufoStartPos.x;
            float y0 = ufoStartPos.y;
            float z0 = ufoStartPos.z;

            // Use p as our curve parameter
            float p = s;  // still 0..1

            // Horizontal motion: z(p) = z0 + rangeZ * p^2 (flipped 180 degrees)
            // -> dz/dp = +2*rangeZ*p, so dz/dp = 0 at p=0 (vertical at launch)
            float z = z0 + rangeZ * p * p;

            // Height: simple “up then slightly level” curve
            // y(p) = y0 + (2*maxHeight*p - maxHeight*p*p)
            // -> dy/dp = 2*maxHeight*(1 - p), positive, so we keep going up
            float y = y0 + (2.f * maxHeight * p - maxHeight * p * p);

            float x = x0; // no sideways motion

            ufoPos = Vec3f{ x, y, z };

            // Direction of motion: forward = tangent of the curve
            float eps = 0.01f;
            float p2 = p + eps;
            if (p2 > 1.0f) p2 = 1.0f;

            float z2 = z0 + rangeZ * p2 * p2;
            float y2 = y0 + (2.f * maxHeight * p2 - maxHeight * p2 * p2);
            float x2 = x0;

            Vec3f ufoPosAhead{ x2, y2, z2 };
            Vec3f vel = ufoPosAhead - ufoPos;
            float speed = length(vel);

            if (speed > 1e-4f)
            {
                forwardWS = vel / speed;

                Vec3f upGuess{ 0.f, 1.f, 0.f };
                if (std::fabs(dot(forwardWS, upGuess)) > 0.99f)
                    upGuess = Vec3f{ 1.f, 0.f, 0.f };

                rightWS = normalize(cross(upGuess, forwardWS));
                upWS    = cross(forwardWS, rightWS);
            }
        }
        else
        {
            // Not active: parked upright on the pad
            ufoPos    = ufoStartPos;
            forwardWS = Vec3f{ 0.f, 1.f, 0.f };
            rightWS   = Vec3f{ 1.f, 0.f, 0.f };
            upWS      = Vec3f{ 0.f, 0.f, 1.f };
        }

		

// ===================
// Orientation from path (no abrupt change)
// ===================

float fy = forwardWS.y;
if (fy > 1.0f)  fy = 1.0f;
if (fy < -1.0f) fy = -1.0f;

// Angles that follow the path
float yawPath   = std::atan2(forwardWS.x, -forwardWS.z);
float pitchPath = std::asin(fy);

// Start angles when idle: rocket is vertical (nose up)
float yawStart   = 0.0f;
float pitchStart = 0.5f * std::numbers::pi_v<float>;

// Blend factor: gradually transition from upright to path-following
// Start at 0 (upright), gradually increase as UFO gains horizontal velocity
float alpha = 0.0f;
if (gUfoAnim.active && u > 0.0f)
{
    // Smooth transition: stays upright early, then follows path
    // Use a steeper curve so it stays upright longer
    alpha = std::min(1.0f, u * u * 3.0f);
}

// Interpolated angles
float ufoYaw   = yawStart   + alpha * (yawPath   - yawStart);
float ufoPitch = pitchStart + alpha * (pitchPath - pitchStart);
float ufoRoll  = 0.0f;

// Orientation matrix using only mat44 helpers
Mat44f ufoOrient =
    make_rotation_y(ufoYaw) *
    make_rotation_x(ufoPitch) *
    make_rotation_z(ufoRoll);

// Mesh correction (as you already had)
Mat44f ufoRot =
    ufoOrient *
    make_rotation_y(std::numbers::pi_v<float>) *
    make_rotation_x(0.5f * std::numbers::pi_v<float>);

// Full model
Mat44f ufoModel =
    make_translation(ufoPos) *
    ufoRot *
    make_scaling(0.5f, 0.5f, 0.5f);



		// Attach point lights to UFO
		gPointLights[0].position = ufoPos + lightOffset0;
		gPointLights[1].position = ufoPos + lightOffset1;
		gPointLights[2].position = ufoPos + lightOffset2;



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
		Vec3f camPosForLighting = gCamera.position;   // default

        switch (gCameraMode)
        {
        case CameraMode::Free:
        {

			camPosForLighting = gCamera.position;

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

			camPosForLighting = camPos;

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

			camPosForLighting = camPos;

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
        Mat44f viewProj = proj * view;

        // Terrain model is identity, so MVP_terrain = viewProj * model
        Mat44f terrainMvp = viewProj * model;

        // UFO MVP
        Mat44f ufoMvp = viewProj * ufoModel;

        // 6) Normal matrix (still fine with model = identity)
		// --- Normal matrices ---
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

// lighting

// Lighting uniforms (same for both objects)
glUniform3fv(2, 1, &lightDir[0]);
glUniform3fv(4, 1, &ambientColor[0]);

// Normal matrix (still fine for both; only uniform scaling)
glUniformMatrix3fv(
    1, // matches layout(location = 1) uniform mat3 uNormalMatrix;
    1, GL_TRUE, normalMatrix.v
);

// Camera position (location = 6)
glUniform3fv(6, 1, &camPosForLighting.x);


// Tell shader to use the texture
glUniform1i(17, 1);  // uUseTexture = 1


// Point lights (locations 7-9, 10-12, 13-15)
Vec3f pointLightPositions[3] = {
    gPointLights[0].position,
    gPointLights[1].position,
    gPointLights[2].position
};
Vec3f pointLightColors[3] = {
    gPointLights[0].color,
    gPointLights[1].color,
    gPointLights[2].color
};
GLint pointLightEnabled[3] = {
    gPointLights[0].enabled ? 1 : 0,
    gPointLights[1].enabled ? 1 : 0,
    gPointLights[2].enabled ? 1 : 0
};

glUniform3fv(7, 3, &pointLightPositions[0].x);   // locations 7, 8, 9
glUniform3fv(10, 3, &pointLightColors[0].x);     // locations 10, 11, 12
glUniform1iv(13, 3, pointLightEnabled);          // locations 13, 14, 15

// Directional light enabled (location = 16)
glUniform1i(16, gDirectionalLightEnabled ? 1 : 0);



// ====================
// Draw TERRAIN
// ====================
glUniformMatrix3fv(
    1,  // location = 1 in the shader
    1, GL_TRUE, normalMatrix.v
);

glUniformMatrix4fv(0, 1, GL_TRUE, terrainMvp.v);
glUniformMatrix4fv(18, 1, GL_TRUE, model.v);   // uModel at location 18

// Terrain colour from your 1.3 work
glUniform3fv(3, 1, &baseColor[0]);


// texture
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, terrainTexture);
glUniform1i(5, 0);

// draw terrain

glBindVertexArray(terrainVAO);
glDrawArrays(GL_TRIANGLES, 0, terrainMeshData.positions.size());
glBindVertexArray(0);

// ====================
// Draw UFO
// ====================

glUniformMatrix3fv(
	1,  // location = 1 in the shader
	1, GL_TRUE, normalMatrix.v
);


glUniformMatrix4fv(18, 1, GL_TRUE, ufoModel.v);   // uModel at location 18

glBindVertexArray(ufoMesh.vao);

// Tell shader to ignore texture and just use uBaseColor
glUniform1i(17, 0);  // uUseTexture = 0

// Base (body + fins)
Vec3f bodyColor{ 0.2f, 0.28f, 0.38f };   // tweak
glUniform3fv(3, 1, &bodyColor[0]);
glUniformMatrix4fv(0, 1, GL_TRUE, ufoMvp.v);
glDrawArrays(GL_TRIANGLES, 0, ufoBaseVertexCount);

// Top (nose + antenna)
Vec3f topColor{ 0.25f, 0.45f, 0.95f };
glUniform3fv(3, 1, &topColor[0]);
glDrawArrays(GL_TRIANGLES, ufoBaseVertexCount, ufoTopVertexCount);


// // 1) Base saucer (grey) : vertices [0, ufoBaseVertexCount)
// glUniform3fv(3, 1, &saucerColor[0]);
// glUniformMatrix4fv(0, 1, GL_TRUE, ufoMvp.v);
// glDrawArrays(GL_TRIANGLES, 0, ufoBaseVertexCount);

// // 2) Top dome + antenna (light blue) : vertices [ufoBaseVertexCount, ufoBaseVertexCount + ufoTopVertexCount)
// glUniform3fv(3, 1, &domeColor[0]);
// glDrawArrays(GL_TRIANGLES, ufoBaseVertexCount, ufoTopVertexCount);

glBindVertexArray(0);


// === Draw landing pad twice (no duplicated data) ===

		GLuint landingProgId = landingProgram.programId();
		glUseProgram(landingProgId);

		// Normal matrix (location = 1 in the vertex shader)
		glUniformMatrix3fv(
			1,
			1, GL_TRUE, normalMatrix.v
		);

		// Lighting uniforms (same as terrain)
		glUniform3fv(2, 1, &lightDir[0]);
		glUniform3fv(4, 1, &ambientColor[0]);
		
		// Camera position (location = 6)
		glUniform3fv(6, 1, &gCamera.position.x);

		// Point lights (locations 7-9, 10-12, 13-15)
		glUniform3fv(7, 3, &pointLightPositions[0].x);   // locations 7, 8, 9
		glUniform3fv(10, 3, &pointLightColors[0].x);     // locations 10, 11, 12
		glUniform1iv(13, 3, pointLightEnabled);          // locations 13, 14, 15

		// Directional light enabled (location = 16)
		glUniform1i(16, gDirectionalLightEnabled ? 1 : 0);

	glBindVertexArray(landingVao);

	// First landing pad
	Mat44f lpModel1 = make_translation(landingPadPos1);
	glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);     // ViewProj at location 0
	glUniformMatrix4fv(17, 1, GL_TRUE, lpModel1.v);    // Model at location 17
	glDrawArrays(GL_TRIANGLES, 0, landingMeshData.positions.size());

	// Second landing pad
	Mat44f lpModel2 = make_translation(landingPadPos2);
	glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);     // ViewProj at location 0
	glUniformMatrix4fv(17, 1, GL_TRUE, lpModel2.v);    // Model at location 17
	glDrawArrays(GL_TRIANGLES, 0, landingMeshData.positions.size());		glBindVertexArray(0);


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

	bool pressed  = (aAction == GLFW_PRESS);

	//SWAP aKey with GLFW ------------------
	if (aKey == GLFW_KEY_ESCAPE && pressed)
	{
		glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
		return;
	}

	// Movement keys //SWAP aKey with GLFW ------------------
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

	// Light toggles: 1–3 toggle point lights, 4 toggles directional light //TASK 1.6 MAKE SURE CONSISTENT WITH UP
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
