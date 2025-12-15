//#version 430 core
// Mac version
#version 410 core

layout(location = 0) in vec3 aPosition;

// Matches your C++ bindings:
//layout(location = 0) uniform mat4 uViewProj;
//layout(location = 1) uniform float uBaseSize;   // C++ still sets this with location 1
//layout(location = 4) uniform vec3 uCameraPosition;   // you already upload this in C++

uniform mat4 uViewProj;
uniform float uBaseSize;   // C++ still sets this with location 1
uniform vec3 uCameraPosition;

void main()
{
    // Transform to clip space
    gl_Position = uViewProj * vec4(aPosition, 1.0);

    // Distance from camera in world space
    float dist = length(aPosition - uCameraPosition);

    // Protect against divide-by-zero
    dist = max(dist, 0.001);

    // Inversely scale size with distance
    float size = 10.0 * uBaseSize / dist;

    // Clamp so they’re never microscopic or gigantic
    gl_PointSize = clamp(size, 0.0, 400.0);
}
