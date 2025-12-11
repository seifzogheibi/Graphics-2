#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

enum class CameraMode
{
    Free = 0,   // free movement
    Chase = 1,  // fixed distance behind and to the side of rocket
    Ground = 2  // fixed point on ground and always looks at the rocket
};

// Camera state
struct Camera
{
    Vec3f position{ 0.f, 5.f, 0.f }; // strarting position (above ground)
    float LRangle   = 0.0f; // horizontal angle
    float UDangle = 0.0f; // vertical angle

    // Movement flags
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;

    // mouse state
    bool mouseCaptured = false;
    bool firstMouse = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    
    // Speed modifiers
    bool fast = false; // shift
    bool slow = false; // ctrl
};

// final view and position result
struct CameraResult
{
    Mat44f view;
    Vec3f position;
};

// Build view matrix depending on the camera mode choice
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