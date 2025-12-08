#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

enum class CameraMode
{
    Free = 0,   // WASD + mouse
    Chase = 1,  // fixed distance behind/above rocket
    Ground = 2  // fixed point on ground, always looks at rocket
};

struct Camera
{
    Vec3f position{ 0.f, 5.f, 0.f };
    float yaw   = 0.0f;
    float pitch = 0.0f;

    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;

    bool fast = false;
    bool slow = false;

    bool mouseCaptured = false;
    bool firstMouse = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};

struct CameraResult
{
    Mat44f view;
    Vec3f position;
};

// Compute view matrix for a given camera mode
CameraResult computeCameraView(
    CameraMode mode,
    Camera const& camera,
    Vec3f const& ufoPos,
    Vec3f const& forwardWS,
    Vec3f const& landingPadPos1
);

// Update camera position based on input
void updateCameraMovement(
    Camera& camera,
    float dt
);

// Handle mouse movement for camera rotation
void handleCameraMouseMovement(
    Camera& camera,
    double xpos,
    double ypos
);

#endif // CAMERA_HPP_INCLUDED