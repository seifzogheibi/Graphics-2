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
    CameraResult result;
    
    switch (mode)
    {
    case CameraMode::Free:
    {
        // Free movement mode
        result.position = camera.position;
        result.view = 
            make_rotation_x(-camera.pitch) * // look up and down
            make_rotation_y(camera.yaw) * // look right and left
            make_translation(-camera.position); // move world opposite to camera
        break;
    }
    case CameraMode::Chase:
    {
        // Chase mode behind and above the spaceship (to the side)
        Vec3f worldUp{ 0.f, 1.f, 0.f };
        Vec3f f = forwardWS;
        
        // Project forward onto horizontal plane
        Vec3f fHoriz{ f.x, 0.f, f.z };
        float len = length(fHoriz);
        if (len < 1e-3f)
            fHoriz = Vec3f{ 0.f, 0.f, -1.f }; // fallback forward
        else
            fHoriz = fHoriz / len; // normalize

        float distBack   = 7.0f; // distance behind spaceship
        float heightUp   = 1.0f; // height above spaceship
        float sideOffset = -2.0f; // offset sideways

        Vec3f right = normalize(cross(worldUp, fHoriz));

        // Camera position
        Vec3f camPos = 
            ufoPos
            - fHoriz * distBack
            + worldUp * heightUp
            + right * sideOffset;

        // Camera looks ahead of spaceship slightly
        Vec3f camTarget = ufoPos + fHoriz * 2.0f;
        Vec3f dir = normalize(camTarget - camPos);

        // convert direction to yaw/pitch
        float camPitch = std::asin(dir.y);
        float camYaw = std::atan2(dir.x, -dir.z);

        result.position = camPos;
        // build view matrix 
        result.view = 
            make_rotation_x(-camPitch) *
            make_rotation_y(camYaw) *
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
        float camPitch = std::asin(dir.y);
        float camYaw = std::atan2(dir.x, -dir.z);

        result.position = camPos;
        result.view = 
            make_rotation_x(-camPitch) *
            make_rotation_y(camYaw) *
            make_translation(-camPos);
        break;
    }
    }
    
    return result;
}

// Update camera position based on input
void updateCameraMovement(Camera& camera, float dt)
{
    // Base movement speed
    float baseSpeed = 5.0f;
    if (camera.fast) baseSpeed *= 4.0f;
    if (camera.slow) baseSpeed *= 0.25f;

    float moveStep = baseSpeed * dt; // movement distance

    // Calculate forward and right vectors from yaw/pitch
    Vec3f camForward{
        std::sin(camera.yaw) * std::cos(camera.pitch),
        std::sin(camera.pitch),
        -std::cos(camera.yaw) * std::cos(camera.pitch)
    };
    camForward = normalize(camForward);

    Vec3f camRight{
        std::cos(camera.yaw),
        0.0f,
        std::sin(camera.yaw)
    };
    camRight = normalize(camRight);

    Vec3f camUp{ 0.0f, 1.0f, 0.0f };

    // Move camera based on input flags
    if (camera.moveForward)
        camera.position = camera.position + camForward * moveStep;
    if (camera.moveBackward)
        camera.position = camera.position - camForward * moveStep;
    if (camera.moveRight)
        camera.position = camera.position + camRight * moveStep;
    if (camera.moveLeft)
        camera.position = camera.position - camRight * moveStep;
    if (camera.moveUp)
        camera.position = camera.position + camUp * moveStep;
    if (camera.moveDown)
        camera.position = camera.position - camUp * moveStep;
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
    camera.yaw   += static_cast<float>(xoffset) * sensitivity;
    camera.pitch -= static_cast<float>(yoffset) * sensitivity;

    // clamp pitch to avoid flipping
    float maxPitch = 1.5f;
    if (camera.pitch > maxPitch)  camera.pitch = maxPitch;
    if (camera.pitch < -maxPitch) camera.pitch = -maxPitch;
}