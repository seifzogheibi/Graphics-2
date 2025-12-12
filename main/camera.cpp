#include "camera.hpp"
#include <cmath>

// Build view matrix and camera position for each mode
CameraResult computeCameraView(
    CameraMode mode,
    Camera const& camera,
    Vec3f const& ufoPos,
    Vec3f const& forwardWS,
    Vec3f const& landingPadPos1
)
{
    CameraResult finalLook;
    switch (mode)
    { case CameraMode::Free:
    {
        // Free movement mode
        finalLook.position = camera.position;
        finalLook.view = 
            make_rotation_x(-camera.yaw) *
            make_rotation_y(camera.pitch) *
            make_translation(-camera.position); // moves world opposite to camera
        break;
    }
    case CameraMode::Chase:
    {
        // Chase mode behind and above the spaceship (to the side)
        Vec3f worldUp{ 0.f, 1.f, 0.f };
        Vec3f f = forwardWS;
        
        // Forward onto horizontal plane
        Vec3f fH{ f.x, 0.f, f.z };
        float len = length(fH);
        Vec3f defaultpos{ 0.f, 0.f, 1.f };
        float blendThreshold = 0.3f;
        if (len < 1e-3f)
            fH = defaultpos;
        else{
            Vec3f fHNormal = fH / len;
        // Smoothes out the camera to stop the shaking
        float blend = std::min(1.0f, len / blendThreshold);
        fH = normalize(defaultpos * (1.0f - blend) + fHNormal * blend);
        }
        float followingdist = 7.0f;
        float followingH = 1.0f;
        float sideoffset = -2.0f;

        Vec3f right = normalize(cross(worldUp, fH));
        Vec3f camPos = ufoPos-fH * followingdist + worldUp * followingH + right * sideoffset;

        // Camera looks ahead of spaceship slightly
        Vec3f camTarget = ufoPos + fH * 2.0f;
        Vec3f dir = normalize(camTarget - camPos);

        // Turn  direction to pitch/yaw
        float camyaw = std::asin(dir.y);
        float campitch = std::atan2(dir.x, -dir.z);

        finalLook.position = camPos;
        finalLook.view = make_rotation_x(-camyaw) *make_rotation_y(campitch) * make_translation(-camPos);
        break;
    }
    case CameraMode::Ground:
    {
        // Fixed camera on ground locked at the spaceship
        Vec3f camPos{
            landingPadPos1.x + 10.f,
            landingPadPos1.y + 1.f,
            landingPadPos1.z + 12.f
        };
        // direction from camera to spaceship
        Vec3f dir = normalize(ufoPos - camPos);
        float camyaw = std::asin(dir.y);
        float campitch = std::atan2(dir.x, -dir.z);

        finalLook.position = camPos;
        finalLook.view = make_rotation_x(-camyaw) *make_rotation_y(campitch) * make_translation(-camPos);
        break;
    }
    }
    return finalLook;
}

// Moves camera position based on the input
void updatedCam(Camera& camera, float dt)
{
    float baseSpeed = 5.0f;
    if (camera.fast) baseSpeed *= 4.0f;
    if (camera.slow) baseSpeed *= 0.25f;
    float moveStep = baseSpeed * dt;

    // Calculate forward and right vectors from pitch/yaw
    Vec3f camForward{std::sin(camera.pitch) ,0.0f, -std::cos(camera.pitch)
    };
    camForward = normalize(camForward);

    Vec3f rightCamera{std::cos(camera.pitch),0.0f, std::sin(camera.pitch)
    };
    rightCamera = normalize(rightCamera);
    Vec3f cameraUp{ 0.0f, 1.0f, 0.0f };

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

// Mouse movement for camera rotation only when mouse is locked
void cameraMouseLook(Camera& camera, double xpos, double ypos)
{
    if (!camera.mouseLocked)
        return;
    if (camera.firstMouse)
    {
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
    float sensitivity = 0.003f;
    camera.pitch   += static_cast<float>(xoffset) * sensitivity;
    camera.yaw -= static_cast<float>(yoffset) * sensitivity;

    // clamp yaw to avoid flipping
    float maxyaw = 1.5f;
    if (camera.yaw > maxyaw)  camera.yaw = maxyaw;
    if (camera.yaw < -maxyaw) camera.yaw = -maxyaw;
}