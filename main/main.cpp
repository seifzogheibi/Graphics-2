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


#include <string>
#include <unordered_map>


#include "measuring_performance.hpp"

// define path to assets folder using a preprocessor macro
#define ASSETS "assets/cw2/"

// namespace to define functions/structs that will be used in this file
namespace
{
    constexpr char const* kWindowTitle = "COMP3811 - CW2";

    // callback error by GLFW that returms error code followed by a message
    void glfw_callback_error_( int, char const* );

    // callback for key presses in the window
    void glfw_callback_key_( GLFWwindow*, int, int, int, int );

    // callback for mouse button presses in the window
    void glfw_callback_mouse_button_( GLFWwindow* , int , int , int );

    // callback for mouse movement in the window
    void glfw_callback_cursor_position_( GLFWwindow* , double, double );

    struct GLFWCleanupHelper
    {
        ~GLFWCleanupHelper();
    };
    struct GLFWWindowDeleter
    {
        ~GLFWWindowDeleter();
        GLFWwindow* window;
    };

    // struct setups before main loops

    // world camera and view modes
    Camera CameraView;

    // standard first perspn free camera
    CameraMode FirstPerson = CameraMode::Free;
    // start with split screen disabled
    bool SplitScreen = false;
    // camera tracking ship when split screen is enabled
    CameraMode Tracking = CameraMode::Chase;

    // Task 1.7 Spaceship animation
    struct Animation
    {
        // start with stationary ship
        bool active = false;
        // toggle between paused and unpaused animation
        bool paused = false;
        // time since animation started
        float time   = 0.f;
    };
    // store animation state in a global variable (seconds unit)
    Animation SpaceshipAnimation;

    // Task 1.10 Particle system
    ParticleSystem Particles;

    // Task 1.12 Performance profiler
    GPUProfiler gProfiler;

    // UI mouse states for buttons (task 1.11)
    // Mouse x ansd y coordinates
    double mouse_x= 0.0;
    double mouse_y = 0.0;
    // start with nothing being clicked
    bool MouseClick = false;

    // Task1.6 Lighting
    struct LocalLightSource
    {
        Vec3f position;
        Vec3f color;
        bool enabled = true;
    };

    // Three colored point lights
    LocalLightSource LocalLight[3];
    // directional LocalLight is on by default
    bool Sunlight = true;        

    // Task 1.7 cubic bezier curve REFERENCE
    // bezier function to calculate position on curve at a time t
    Vec3f bezier(
        // start point
        Vec3f const& A,
        // point before curve
        Vec3f const& B,
        // point at end of the curve
        Vec3f const& C,
        // end point
        Vec3f const& D,
        // time parameter 0 to 1
        float t
    )
    {

        // compute bezier formula coefficients
        float it  = 1.f - t;
        float it2 = it * it;
        float t2  = t * t;

        // return bezier function
        return
            (it2*it) * A +
            (3.f* it2*t)* B +
            (3.f*it* t2) *C +
            (t2*t)* D;
    }

    // Draw everything in the program (terrain, spaceship, pads, particles)
    void renderWorld(
        Mat44f const& view_projection,
        Vec3f const& Camera_Lighting,
        GLuint terrain_vao,
        SimpleMeshData const& TerrainMesh,
        GLuint Texture,
        ShaderProgram const& terrain_shader,
        ShaderProgram const& spaceship_shader,
        Mat44f const& model,
        Vec3f const& light_direction,
        Vec3f const& ambience,
        Vec3f const& color,
        GLuint spaceshipVao,
        GLsizei spaceshipVertexCount,
        Mat44f const& SpaceshipMatrix,
        GLuint landing_vao,
        SimpleMeshData const& LandingMesh,
        ShaderProgram const& landing_shader,
        Vec3f const& landing_position,
        Vec3f const& landing2_position,
        ShaderProgram const& particle_shader,
        GPUProfiler& profiler,
        bool doProfile = true
    )
    {
        // mvp matrices to transform vertices from model space into cliped space
        Mat44f terrain_mvp   = view_projection * model;
        Mat44f spaceship_mvp       = view_projection * SpaceshipMatrix;

        // normal matrix is inverse,transpose of model matrix (4x4 to 3x3 because no translation needed for normals)
        Mat33f transform_model = mat44_to_mat33(transpose(invert(model)));

        // place 3 local light sources in world positions
        Vec3f LocalLightPosition[3] = {
            LocalLight[0].position,
            LocalLight[1].position,
            LocalLight[2].position
        };
        // colors of the local light sources
        Vec3f LocalLightColor[3] = {
            LocalLight[0].color,
            LocalLight[1].color,
            LocalLight[2].color
        };
        // enabled states of the local light sources (boolean into int)
        GLint LocalLightOn[3] = {
            LocalLight[0].enabled ? 1 : 0,
            LocalLight[1].enabled ? 1 : 0,
            LocalLight[2].enabled ? 1 : 0
        };
        

        // render terrain
        GLuint shader_main = terrain_shader.programId();
        glUseProgram(shader_main);

        // set uniform locations for terrain shader
        // first number is layout location in shader, second is how many values, last is a pointer to data, (for matrices uses GL_TRUE to transpose)
        glUniform3fv(2, 1, &light_direction.x);
        glUniform3fv(4, 1, &ambience.x);
        glUniformMatrix3fv(1, 1, GL_TRUE, transform_model.v);
        glUniform3fv(6, 1, &Camera_Lighting.x);
        glUniform1i(17, 1);

        // local lights positions, colors, enabled
        glUniform3fv(7, 3, &LocalLightPosition[0].x);
        glUniform3fv(10, 3, &LocalLightColor[0].x);
        glUniform1iv(13, 3, LocalLightOn);
        glUniform1i(16, Sunlight ? 1 : 0);

        // set terrain matrix and color
        glUniformMatrix4fv(0, 1, GL_TRUE, terrain_mvp.v);
        glUniformMatrix4fv(18, 1, GL_TRUE, model.v);
        glUniform3fv(3, 1, &color.x);

        // bind terrain texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glUniform1i(5, 0);

        // terrain rendering
        glBindVertexArray(terrain_vao);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)TerrainMesh.positions.size());
        glBindVertexArray(0);
        // stamp end of terrain rendering
        gpuStamp(profiler, Stamp::TerrainEnd, doProfile);

        GLuint shipProg = spaceship_shader.programId();
        glUseProgram(shipProg);
        
        glUniform3fv(2, 1, &light_direction.x);
        glUniform3fv(4, 1, &ambience.x);
        glUniform3fv(6, 1, &Camera_Lighting.x);

        glUniform3fv(7, 3, &LocalLightPosition[0].x);
        glUniform3fv(10, 3, &LocalLightColor[0].x);
        glUniform1iv(13, 3, LocalLightOn);
        glUniform1i(16, Sunlight ? 1 : 0);
        
        // spaceship rendering, set uniforms
        glUniformMatrix3fv(1, 1, GL_TRUE, transform_model.v);
        glUniformMatrix4fv(18, 1, GL_TRUE, SpaceshipMatrix.v);
        glBindVertexArray(spaceshipVao);

        // set spaceship matrix
        glUniformMatrix4fv(0, 1, GL_TRUE, spaceship_mvp.v);
        
        // draw spaceship
        glDrawArrays(GL_TRIANGLES, 0, spaceshipVertexCount);

        // unbind spaceship VAO
        glBindVertexArray(0);

        // stamp end of spaceship rendering
        gpuStamp(profiler, Stamp::SpaceshipEnd, doProfile); 
        
        // landing pad rendering
        GLuint landing_id = landing_shader.programId();
        glUseProgram(landing_id);
        glUniformMatrix3fv(1, 1, GL_TRUE, transform_model.v);
        glUniform3fv(2, 1, &light_direction.x);
        glUniform3fv(4, 1, &ambience.x);
        glUniform3fv(6, 1, &Camera_Lighting.x);
        glUniform3fv(7, 3, &LocalLightPosition[0].x);
        glUniform3fv(10, 3, &LocalLightColor[0].x);
        glUniform1iv(13, 3, LocalLightOn);
        glUniform1i(16, Sunlight ? 1 : 0);

        // bind landing pad VAO
        glBindVertexArray(landing_vao);

        // first pad, create model matrix and set uniforms
        Mat44f lpm = make_translation(landing_position);
        glUniformMatrix4fv(0, 1, GL_TRUE, view_projection.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpm.v);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)LandingMesh.positions.size());

        // second pad
        Mat44f lpm2 = make_translation(landing2_position);
        glUniformMatrix4fv(0, 1, GL_TRUE, view_projection.v);
        glUniformMatrix4fv(17, 1, GL_TRUE, lpm2.v);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)LandingMesh.positions.size());

        glBindVertexArray(0);

        gpuStamp(profiler, Stamp::PadsEnd, doProfile);

        // Particle rendering
        renderParticles(Particles,particle_shader.programId(),view_projection.v,Camera_Lighting);
    }

}


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
    glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_DEPTH_BITS,24);

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
    glfwSetCursorPosCallback( window, &glfw_callback_cursor_position_ );
    glfwSetKeyCallback( window, &glfw_callback_key_ );

    glfwMakeContextCurrent( window );
    glfwSwapInterval(1); // vsync on

    // GLAD (OpenGL loader)
    if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
        throw Error( "gladLoadGLLoader() failed - cannot load GL API!");

    std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
    std::print( "VENDOR {}\n", (char const*)glGetString( GL_VENDOR ) );
    std::print( "VERSION {}\n",(char const*)glGetString( GL_VERSION ) );
    std::print( "SHADING_LANGUAGE_VERSION {}\n", (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

#   if !defined(NDEBUG)
    setup_gl_debug_output();
#   endif

    // Initialize performance profiler (Task 1.12)
    gpuInit(gProfiler);

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
    SimpleMeshData TerrainMesh = load_wavefront_obj("assets/cw2/parlahti.obj");
    GLuint terrain_vao = create_vao(TerrainMesh);

    ShaderProgram terrain_shader({
        { GL_VERTEX_SHADER, "assets/cw2/default.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
    });

    // World transform for terrain
    Mat44f model = kIdentity44f;

    // Task1.6 Lighting colors
    Vec3f light_direction = normalize(Vec3f{ 0.f, 1.f, -1.f }); // light direction
    Vec3f color = { 0.6f, 0.7f, 0.6f };
    Vec3f ambience = { 0.18f, 0.18f, 0.18f };

    // Building spaceship mesh once instead of every frame
    spaceshipMesh Spaceship = create_spaceship_mesh();
    GLuint SpaceshipVao = Spaceship.vao;
    GLsizei SpaceshipVertexCount = Spaceship.vertexCount;
    float bulbsHeight = Spaceship.bulbsHeight;
    float bulbRadius = Spaceship.bulbRadius;

    GLuint terrainTexture = load_texture_2d( (ASSETS + TerrainMesh.texture_filepath).c_str() );

    // Landing pad shader and mesh
    ShaderProgram landing_shader({
        { GL_VERTEX_SHADER, "assets/cw2/landing.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/landing.frag" }
    });

    Vec3f landing_position{-11.50f, -0.96f, -54.f};
    Vec3f landing2_position{ 8.f, -0.96f, 40.f };
    
    SimpleMeshData LandingMesh = load_wavefront_obj("assets/cw2/landingpad.obj");
    GLuint landing_vao = create_vao(LandingMesh);

    ShaderProgram spaceship_shader({
        { GL_VERTEX_SHADER, "assets/cw2/spaceship.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/spaceship.frag" }
    });
    
    // UI shader and renderer
    ShaderProgram uiShader({
        {GL_VERTEX_SHADER,"assets/cw2/ui.vert"},
        {GL_FRAGMENT_SHADER,"assets/cw2/ui.frag"}
    });
    // Window size gets updated every frame in the main loop
    UIRenderer uiRenderer(1280, 720, uiShader); 

    Button launchButton{"Launch", 0, 0, 120, 40, // this way the buttons stays centered
        {0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    Button resetButton{"Reset", 0, 0, 120, 40,
        {0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

    // Point lights attached to the spaceship
    LocalLight[0].color = Vec3f{ 1.f, 0.f, 0.f }; //red
    LocalLight[1].color = Vec3f{ 0.f, 1.f, 0.f }; // green
    LocalLight[2].color = Vec3f{ 0.f, 0.f, 1.f }; // blue
    LocalLight[0].enabled = true;
    LocalLight[1].enabled = true;
    LocalLight[2].enabled = true;
    Sunlight = true;

    // Particle shader
    ShaderProgram particle_shader({
        {GL_VERTEX_SHADER, "assets/cw2/particle.vert"},
        {GL_FRAGMENT_SHADER, "assets/cw2/particle.frag"}
    });

    initialize_ParticleSystem(Particles, "assets/cw2/particle.png");

    OGL_CHECKPOINT_ALWAYS();

    // Main loop
    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();

        // Keep framebuffer size updated
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
                { glfwWaitEvents();
                    glfwGetFramebufferSize( window, &nwidth, &nheight );
                } while( 0 == nwidth || 0 == nheight );
            }
            glViewport( 0, 0, nwidth, nheight );
        }

        // Calculate delta time since last frame to make movement smooth
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        // Update animation time if active and not paused
        if (SpaceshipAnimation.active && !SpaceshipAnimation.paused)
            SpaceshipAnimation.time += dt;

        // Task 1.7 camera projection 
        float aspect = fbwidth / fbheight;
        float fovRadians = 60.0f * std::numbers::pi_v<float> / 180.0f;
        float zNear = 0.1f;
        float zFar = 250.0f;

        Mat44f proj = make_perspective_projection(fovRadians, aspect, zNear, zFar);

        // spaceship above first landing pad
        Vec3f spaceship_start_position = {
            landing_position.x,
            landing_position.y + 1.3f,
            landing_position.z
        };
        float lightRadius = bulbRadius;
        Vec3f lightOffset0{  lightRadius, bulbsHeight - 0.35f, 0.0f };
        Vec3f lightOffset1{ -0.5f * lightRadius, bulbsHeight - 0.35f,0.866025f * lightRadius };
        Vec3f lightOffset2{ -0.5f * lightRadius, bulbsHeight - 0.35f,-0.866025f * lightRadius };

        // inital spaceship before launching (erect on landing pad)
        Vec3f spaceship_current_position = spaceship_start_position;
        Vec3f forwardWS{ 0.f, 1.f, 0.f };
        Vec3f rightWS{ 1.f, 0.f, 0.f };
        Vec3f upWS { 0.f, 0.f, 1.f };
        float u = 0.0f;

        if (SpaceshipAnimation.active)
        {
            float totalTime = 12.0f; // duration of animation
            float tAnim = SpaceshipAnimation.time;
            if (tAnim < 0.f)       tAnim = 0.f;
            if (tAnim > totalTime) tAnim = totalTime;

            float s = tAnim / totalTime;
            u = s * s; // makes takeoff look smoother
            float rangeZ    = 140.0f;
            float maxHeight = 80.0f;
            float x0 = spaceship_start_position.x;
            float y0 = spaceship_start_position.y;
            float z0 = spaceship_start_position.z;

            // Bezier controls for curved take off path
            Vec3f A = spaceship_start_position;
            Vec3f B{ x0, y0 + maxHeight * 0.7f,z0 };
            Vec3f C{ x0, y0 + maxHeight, z0 + rangeZ * 0.55f };
            Vec3f D{ x0, y0 + maxHeight * 0.2f, z0 + rangeZ };

            spaceship_current_position = bezier(A, B, C, D, u);

            // approximates tangent by sampling slightly ahead on the curve
            float eps = 0.001f;
            float u2 = u + eps;
            if (u2 > 1.0f) u2 = 1.0f;

            Vec3f posAhead = bezier(A, B, C, D, u2);
            Vec3f vel = posAhead - spaceship_current_position;
            float speed = length(vel);

            if (speed > 1e-4f)
            {
                forwardWS = vel / speed;
                Vec3f worldUp{ 0.f, 1.f, 0.f };
                if (std::fabs(dot(forwardWS, worldUp)) > 0.99f)
                worldUp = Vec3f{ 1.f, 0.f, 0.f };
                rightWS = normalize(cross(worldUp, forwardWS));
                upWS = cross(forwardWS, rightWS);
            }
        }
        else
        {
            // Space ship pointing upwards on landing pad (idle)
            spaceship_current_position = spaceship_start_position;
            forwardWS = Vec3f{ 0.f, 1.f, 0.f };
            rightWS = Vec3f{ 1.f, 0.f, 0.f };
            upWS = Vec3f{ 0.f, 0.f, 1.f };
        }

        // Rotate spaceship to follow path direction
        float fy = std::clamp(forwardWS.y, -1.0f, 1.0f);
        float spaceship_yaw = std::atan2(forwardWS.x, -forwardWS.z);
        float spaceship_pitch = std::asin(fy);
        float spaceship_roll  = 0.0f;

        Mat44f spaceship_orientation =
            make_rotation_y(spaceship_yaw) * make_rotation_x(spaceship_pitch) * make_rotation_z(spaceship_roll);

        // Rotates the spaceships axes to the world axes
        Mat44f spaceship_rotation =
            spaceship_orientation * make_rotation_y(std::numbers::pi_v<float>) * 
            make_rotation_x(0.5f * std::numbers::pi_v<float>);

        Mat44f SpaceshipMatrix =
            make_translation(spaceship_current_position) * spaceship_rotation * make_scaling(0.5f, 0.5f, 0.5f);

            // Rotate the light offsets so bulbs stay on the spaceship when it turns
            Vec4f w0 = spaceship_rotation * Vec4f{ lightOffset0.x, lightOffset0.y, lightOffset0.z, 0.f };
            Vec4f w1 = spaceship_rotation * Vec4f{ lightOffset1.x, lightOffset1.y, lightOffset1.z, 0.f };
            Vec4f w2 = spaceship_rotation * Vec4f{ lightOffset2.x, lightOffset2.y, lightOffset2.z, 0.f };
            lightOffset0 = Vec3f{ w0.x, w0.y, w0.z };
            lightOffset1 = Vec3f{ w1.x, w1.y, w1.z };
            lightOffset2 = Vec3f{ w2.x, w2.y, w2.z };

            // Attach point lights to the spaceship's body
            LocalLight[0].position = spaceship_current_position + lightOffset0;
            LocalLight[1].position = spaceship_current_position + lightOffset1;
            LocalLight[2].position = spaceship_current_position + lightOffset2;

        // Task 1.10 Particles    
        static bool  firstEngineFrame = true;
        static Vec3f previous_engine_position{};
        if (SpaceshipAnimation.active && !SpaceshipAnimation.paused)
        {
            Vec3f current_engine_position = spaceship_current_position - forwardWS * 1.2f;

            if (firstEngineFrame)
            {
                // On the first frame the previous position is the current position
                previous_engine_position = current_engine_position;
                firstEngineFrame = false;
            }
            emitParticles( Particles, dt, previous_engine_position,current_engine_position,forwardWS,rightWS,upWS);
            previous_engine_position = current_engine_position;
        }
        else
        {
            // reset first frame flag if paused or reset
            firstEngineFrame = true;
        }
        if (!SpaceshipAnimation.paused)
        { 
            updateParticles(Particles, dt);
        }

        // Camera movement
        updatedCam(CameraView, dt);

        // Keep the buttons in the bottom center of the screen
        float buttonY = fbheight - 60.0f;
        launchButton.x = fbwidth / 2.0f - 70.0f;
        launchButton.y = buttonY;
        resetButton.x  = fbwidth / 2.0f + 70.0f;
        resetButton.y  = buttonY;

        uploadParticleData(Particles);

        // Rendering 
        OGL_CHECKPOINT_DEBUG();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start GPU and CPU timing
       gpuBegin(gProfiler);
       cpuSubmitBegin(gProfiler);
        if (!SplitScreen)
        {
            // Fullscreen single view
            glViewport(0, 0,(int)fbwidth,(int)fbheight);

            CameraResult camResult = computeCameraView(FirstPerson, CameraView, spaceship_current_position,
                forwardWS, landing_position );

            Mat44f view_projection = proj * camResult.view;

            renderWorld(
                view_projection,
                camResult.position,
                terrain_vao,
                TerrainMesh,
                terrainTexture,
                terrain_shader,
                spaceship_shader,
                model,
                light_direction,
                ambience,
                color,
                SpaceshipVao,
                SpaceshipVertexCount,
                SpaceshipMatrix,
                landing_vao,
                LandingMesh,
                landing_shader,
                landing_position,
                landing2_position,
                particle_shader,
                gProfiler
            );
        }
        else
        {
            // Task 1.9
            // Split screen left and right views
            int leftWidth  = (int)fbwidth / 2;
            int rightWidth = (int)fbwidth - leftWidth;
            int fullHeight = (int)fbheight;

            // Left view (main camera)
            glViewport(0, 0, leftWidth, fullHeight);
            float aspectLeft = float(leftWidth) / float(fullHeight);
            Mat44f projLeft = make_perspective_projection(fovRadians, aspectLeft, zNear, zFar);

            CameraResult camResult1 = computeCameraView(FirstPerson, CameraView, spaceship_current_position,
                forwardWS, landing_position );

            Mat44f viewProj1 = projLeft * camResult1.view;

            renderWorld(
                viewProj1,
                camResult1.position,
                terrain_vao,
                TerrainMesh,
                terrainTexture,
                terrain_shader,
                spaceship_shader,
                model,
                light_direction,
                ambience,
                color,
                SpaceshipVao,
                SpaceshipVertexCount,
                SpaceshipMatrix,
                landing_vao,
                LandingMesh,
                landing_shader,
                landing_position,
                landing2_position,
                particle_shader,
                gProfiler, true
            );

            // Right view 
            glViewport(leftWidth, 0, rightWidth, fullHeight);
            float aspectRight = float(rightWidth) / float(fullHeight);
            Mat44f projRight = make_perspective_projection(fovRadians, aspectRight, zNear, zFar);

            CameraResult camResult2 = computeCameraView(Tracking, CameraView, spaceship_current_position,
                forwardWS, landing_position );

            Mat44f viewProj2 = projRight * camResult2.view;

            renderWorld(
                viewProj2,
                camResult2.position,
                terrain_vao,
                TerrainMesh,
                terrainTexture,
                terrain_shader,
                spaceship_shader,
                model,
                light_direction,
                ambience,
                color,
                SpaceshipVao,
                SpaceshipVertexCount,
                SpaceshipMatrix,
                landing_vao,
                LandingMesh,
                landing_shader,
                landing_position,
                landing2_position,
                particle_shader,
                gProfiler, false // so does not stamp terrain twice
            );

            // Reset viewport to full size for UI
            glViewport(0, 0, (int)fbwidth, (int)fbheight);
        }

        // End GPU and CPU timing
        cpuSubmitEnd(gProfiler);
        gpuStamp(gProfiler, Stamp::FrameEnd);
        gpuCollectResults(gProfiler);

        // Draws UI altitude and buttons
        uiRenderer.setWindowSize((int)fbwidth, (int)fbheight);
        uiRenderer.beginFrame();

        // Altitude in top left
        char altitudeText[64];
        std::snprintf(altitudeText, sizeof(altitudeText),"Altitude: %.1f m", spaceship_current_position.y);
        uiRenderer.renderText( 10.0f, 10.0f, altitudeText, 24.0f,
            Vec4f{1.0f, 1.0f, 1.0f, 1.0f}
        );

        // Launch button that starts if not active and pauses if active
        if (uiRenderer.renderButton(launchButton, mouse_x, mouse_y, MouseClick))
        {
            if (!SpaceshipAnimation.active)
            {
                SpaceshipAnimation.active = true;
                SpaceshipAnimation.paused = false;
                SpaceshipAnimation.time = 0.f;
            }
            else
            {
                SpaceshipAnimation.paused = !SpaceshipAnimation.paused;
            }
        }

        // Reset button that sstops animation and resets particles
        if (uiRenderer.renderButton(resetButton, mouse_x, mouse_y, MouseClick))
        {
            SpaceshipAnimation.active = false;
            SpaceshipAnimation.paused = false;
            SpaceshipAnimation.time = 0.f;
            resetParticles(Particles);
        }
        uiRenderer.endFrame();

        glfwSwapBuffers( window );
    }
    gpuDestroy(gProfiler);
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
            CameraView.moveForward = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_S)
            CameraView.moveBackward = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_A)
            CameraView.moveLeft = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_D)
            CameraView.moveRight = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_E)
            CameraView.moveUp = pressed || (aAction == GLFW_REPEAT);
        else if (aKey == GLFW_KEY_Q)
            CameraView.moveDown = pressed || (aAction == GLFW_REPEAT);

        // Light toggles
        if (aAction == GLFW_PRESS)
        {
            if (aKey == GLFW_KEY_1)
                LocalLight[0].enabled = !LocalLight[0].enabled;
            else if (aKey == GLFW_KEY_2)
                LocalLight[1].enabled = !LocalLight[1].enabled;
            else if (aKey == GLFW_KEY_3)
                LocalLight[2].enabled = !LocalLight[2].enabled;
            else if (aKey == GLFW_KEY_4)
                Sunlight = !Sunlight;
        }

        // Speed modifiers
        if (aKey == GLFW_KEY_LEFT_SHIFT || aKey == GLFW_KEY_RIGHT_SHIFT)
            CameraView.fast = pressed || (aAction == GLFW_REPEAT);
        if (aKey == GLFW_KEY_LEFT_CONTROL || aKey == GLFW_KEY_RIGHT_CONTROL)
            CameraView.slow = pressed || (aAction == GLFW_REPEAT);

        // Spaceship animation controls (keyboard)
        if (aKey == GLFW_KEY_F && aAction == GLFW_PRESS)
        {
            if (!SpaceshipAnimation.active)
            {
                SpaceshipAnimation.active = true;
                SpaceshipAnimation.paused = false;
                SpaceshipAnimation.time   = 0.f;
            }
            else
            {
                SpaceshipAnimation.paused = !SpaceshipAnimation.paused;
            }
        }

        if (aKey == GLFW_KEY_R && aAction == GLFW_PRESS)
        {
            SpaceshipAnimation.active = false;
            SpaceshipAnimation.paused = false;
            SpaceshipAnimation.time   = 0.f;
            resetParticles(Particles);
        }

        // Toggles splitscreen with V
        if (aKey == GLFW_KEY_V && aAction == GLFW_PRESS)
            SplitScreen = !SplitScreen;

        // Camera mode cycling (C for left and Shift+C for right)
        if (aKey == GLFW_KEY_C && aAction == GLFW_PRESS)
        {
            bool shiftPressed =
                (glfwGetKey(aWindow, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS ||
                 glfwGetKey(aWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

            if (shiftPressed)
            {
                // Cycle secondary camera mode
                if (Tracking == CameraMode::Free)
                    Tracking = CameraMode::Chase;
                else if (Tracking == CameraMode::Chase)
                    Tracking = CameraMode::Ground;
                else
                    Tracking = CameraMode::Free;
            }
            else
            {
                // Cycle main camera mode
                if (FirstPerson == CameraMode::Free)
                    FirstPerson  = CameraMode::Chase;
                else if (FirstPerson == CameraMode::Chase)
                    FirstPerson  = CameraMode::Ground;
                else
                    FirstPerson  = CameraMode::Free;
            }
        }
    }

    void glfw_callback_mouse_button_( GLFWwindow* window, int button, int action, int )
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            MouseClick = (action == GLFW_PRESS);

        // Right mouse button toggles mouse capture for camera look
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            CameraView.mouseLocked = !CameraView.mouseLocked;

            if (CameraView.mouseLocked)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                CameraView.firstMouse = true;
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    void glfw_callback_cursor_position_( GLFWwindow*, double xpos, double ypos )
    {
        mouse_x = xpos;
        mouse_y = ypos;

        cameraMouseLook(CameraView, xpos, ypos);
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
