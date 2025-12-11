#include "camera.hpp"
#include <cmath>

// Build view matrix for each mode
CameraResult computeCameraView(
    CameraMode mode,
    Camera const& camera,
    Vec3f const& ufoPos,
    Vec3f const& forwardWS,
    Vec3f const& landingPadPos1
)
{
    CameraResult camfinalresult;
    
    switch (mode)
    {
    case CameraMode::Free:
    {
        // Free movement mode
        camfinalresult.position = camera.position;
        camfinalresult.view = 
            make_rotation_x(-camera.UDangle) * // look up and down
            make_rotation_y(camera.LRangle) * // look right and left
            make_translation(-camera.position); // move world opposite to camera
        break;
    }
    case CameraMode::Chase:
    {
        // Chase mode behind and above the spaceship (to the side)
        Vec3f worldUp{ 0.f, 1.f, 0.f };
        Vec3f f = forwardWS;
        
        // Project forward onto horizontal plane
        Vec3f fH{ f.x, 0.f, f.z };
        float len = length(fH);

        Vec3f defaultpos{ 0.f, 0.f, 1.f }; // default direction when rocket is vertical
        float blendThreshold = 0.3f; // threshold for blending
        if (len < 1e-3f)
            fH = defaultpos;
        else{
            Vec3f calculatedirecton = fH / len; // normalize

        float blend = std::min(1.0f, len / blendThreshold);
        
        // Mix between default and actual forward direction
        fH = normalize(defaultpos * (1.0f - blend) + calculatedirecton * blend);
        }

        float followingdist = 7.0f; // distance behind spaceship
        float followingH = 1.0f; // height above spaceship
        float sideoffset = -2.0f; // offset sideways

        Vec3f right = normalize(cross(worldUp, fH));

        // Camera position
        Vec3f camPos = 
            ufoPos
            - fH * followingdist
            + worldUp * followingH
            + right * sideoffset;

        // Camera looks ahead of spaceship slightly
        Vec3f camTarget = ufoPos + fH * 2.0f;
        Vec3f dir = normalize(camTarget - camPos);

        // Turn  direction to LRangle/UDangle
        float camUDangle = std::asin(dir.y);
        float camLRangle = std::atan2(dir.x, -dir.z);

        camfinalresult.position = camPos;
        // build view matrix 
        camfinalresult.view = 
            make_rotation_x(-camUDangle) *
            make_rotation_y(camLRangle) *
            make_translation(-camPos);
        break;
    }
    case CameraMode::Ground:
    {
        // fixed camera on ground looking at spaceship
        Vec3f camPos{
            landingPadPos1.x + 10.f,
            landingPadPos1.y + 1.f,
            landingPadPos1.z + 12.f
        };

        // direction from camera to spaceship
        Vec3f dir = normalize(ufoPos - camPos);
        float camUDangle = std::asin(dir.y);
        float camLRangle = std::atan2(dir.x, -dir.z);

        camfinalresult.position = camPos;
        camfinalresult.view = 
            make_rotation_x(-camUDangle) *
            make_rotation_y(camLRangle) *
            make_translation(-camPos);
        break;
    }
    }
    
    return camfinalresult;
}

// Update camera position based on input
void updateCameraMovement(Camera& camera, float dt)
{
    // Base movement speed
    float baseSpeed = 5.0f;
    if (camera.fast) baseSpeed *= 4.0f;
    if (camera.slow) baseSpeed *= 0.25f;

    float moveStep = baseSpeed * dt; // movement distance

    // Calculate forward and right vectors from LRangle/UDangle
    Vec3f camForward{
        std::sin(camera.LRangle) ,
        0.0f,
        -std::cos(camera.LRangle)
    };
    camForward = normalize(camForward);

    Vec3f rightCamera{
        std::cos(camera.LRangle),
        0.0f,
        std::sin(camera.LRangle)
    };
    rightCamera = normalize(rightCamera);

    Vec3f cameraUp{ 0.0f, 1.0f, 0.0f };

    // Move camera in each direction based on flags
    if (camera.moveForward)
        camera.position = camera.position + camForward * moveStep;
    if (camera.moveBackward)
        camera.position = camera.position - camForward * moveStep;
    if (camera.moveRight)
        camera.position = camera.position + rightCamera * moveStep;
    if (camera.moveLeft)
        camera.position = camera.position - rightCamera * moveStep;
    if (camera.moveUp)
        camera.position = camera.position + cameraUp * moveStep;
    if (camera.moveDown)
        camera.position = camera.position - cameraUp * moveStep;
}

// Handle mouse movement for camera rotation
void handleCameraMouseMovement(Camera& camera, double xpos, double ypos)
{
    if (!camera.mouseCaptured)
        return;

    if (camera.firstMouse)
    {
        // initialize last  mouse positions
        camera.lastMouseX = xpos;
        camera.lastMouseY = ypos;
        camera.firstMouse = false;
        return;
    }

    // calculates offsets
    double xoffset = xpos - camera.lastMouseX;
    double yoffset = ypos - camera.lastMouseY;
    camera.lastMouseX = xpos;
    camera.lastMouseY = ypos;

    float sensitivity = 0.0025f;
    camera.LRangle   += static_cast<float>(xoffset) * sensitivity;
    camera.UDangle -= static_cast<float>(yoffset) * sensitivity;

    // clamp UDangle to avoid flipping
    float maxUDangle = 1.5f;
    if (camera.UDangle > maxUDangle)  camera.UDangle = maxUDangle;
    if (camera.UDangle < -maxUDangle) camera.UDangle = -maxUDangle;
}