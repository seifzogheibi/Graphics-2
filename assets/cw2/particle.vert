#version 430 core

layout(location = 0) in vec3 aPosition;

layout(location = 0) uniform mat4 uViewProj;
layout(location = 1) uniform float uPointSize;

void main()
{
<<<<<<< HEAD
    gl_Position  = uViewProj * vec4(aPosition, 1.0);
    gl_PointSize = uPointSize;
}
=======
    gl_Position = uViewProj * vec4(aPosition, 1.0);

    // Distance from camera in world space
    float dist = length(aPosition - uCameraPos);

    // Inversely scale size with distance
    float size = 10.0 * uBaseSize / (dist);

    // Clamp so they’re never microscopic or gigantic
    float minSize = 0.0;
    float maxSize = 400.0;
    gl_PointSize = clamp(size, minSize, maxSize);
}
>>>>>>> 7f5b858 (updated)
