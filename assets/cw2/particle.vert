#version 430 core

layout(location = 0) in vec3 aPosition;

layout(location = 0) uniform mat4 uViewProj;
layout(location = 1) uniform float uBaseSize;    // some base size
layout(location = 4) uniform vec3 uCameraPos;    // world-space camera position

void main()
{
    gl_Position = uViewProj * vec4(aPosition, 1.0);

    // Distance from camera in world space
    float dist = length(aPosition - uCameraPos);

    // Inversely scale size with distance
    float size = uBaseSize / dist;

    // Clamp so they’re never microscopic or gigantic
    float minSize = 4.0;
    float maxSize = 40.0;
    gl_PointSize = clamp(size, minSize, maxSize);
}
