#include "camera.hpp"
#include <cmath>

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
        result.position = camera.position;
        result.view = 
            make_rotation_x(-camera.pitch) *
            make_rotation_y(camera.yaw) *
            make_translation(-camera.position);
        break;
    }
    case CameraMode::Chase:
    {
        Vec3f worldUp{ 0.f, 1.f, 0.f };
        Vec3f f = forwardWS;
        
        Vec3f fHoriz{ f.x, 0.f, f.z };
        float len = length(fHoriz);
        if (len < 1e-3f)
            fHoriz = Vec3f{ 0.f, 0.f, -1.f };
        else
            fHoriz = fHoriz / len;

        float distBack   = 7.0f;
        float heightUp   = 1.0f;
        float sideOffset = -2.0f;

        Vec3f right = normalize(cross(worldUp, fHoriz));
        Vec3f camPos = 
            ufoPos
            - fHoriz * distBack
            + worldUp * heightUp
            + right * sideOffset;

        Vec3f camTarget = ufoPos + fHoriz * 2.0f;
        Vec3f dir = normalize(camTarget - camPos);

        float camPitch = std::asin(dir.y);
        float camYaw = std::atan2(dir.x, -dir.z);

        result.position = camPos;
        result.view = 
            make_rotation_x(-camPitch) *
            make_rotation_y(camYaw) *
            make_translation(-camPos);
        break;
    }
    case CameraMode::Ground:
    {
        Vec3f camPos{
            landingPadPos1.x + 10.f,
            landingPadPos1.y + 1.f,
            landingPadPos1.z + 12.f
        };

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

void updateCameraMovement(Camera& camera, float dt)
{
    float baseSpeed = 5.0f;
    if (camera.fast) baseSpeed *= 4.0f;
    if (camera.slow) baseSpeed *= 0.25f;

    float moveStep = baseSpeed * dt;

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

void handleCameraMouseMovement(Camera& camera, double xpos, double ypos)
{
    if (!camera.mouseCaptured)
        return;

    if (camera.firstMouse)
    {
        camera.lastMouseX = xpos;
        camera.lastMouseY = ypos;
        camera.firstMouse = false;
        return;
    }

    double xoffset = xpos - camera.lastMouseX;
    double yoffset = ypos - camera.lastMouseY;
    camera.lastMouseX = xpos;
    camera.lastMouseY = ypos;

    float sensitivity = 0.0025f;
    camera.yaw   += static_cast<float>(xoffset) * sensitivity;
    camera.pitch -= static_cast<float>(yoffset) * sensitivity;

    float maxPitch = 1.5f;
    if (camera.pitch > maxPitch)  camera.pitch = maxPitch;
    if (camera.pitch < -maxPitch) camera.pitch = -maxPitch;
}