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

// Camera state for the free camera
struct Camera
{
    Vec3f position{ 0.f, 5.f, 0.f };
    float pitch = 0.0f;
    float yaw = 0.0f;
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    bool mouseLocked = false;
    bool firstMouse = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool fast = false; // shift
    bool slow = false; // ctrl
};

// final view and position result
struct CameraResult
{ Mat44f view; Vec3f position;
};

// Build view matrix depending on the current camera mode 
CameraResult computeCameraView(
    CameraMode mode,
    Camera const& camera,
    Vec3f const& ufoPos,
    Vec3f const& forwardWS,
    Vec3f const& landingPadPos1
);

// Moves the camera position based on the input
void updatedCam(
    Camera& camera,
    float dt
);
// Mouse movement for camera rotation
void cameraMouseLook(
    Camera& camera,
    double xpos, double ypos
);
#endif