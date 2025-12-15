#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 0) uniform mat4 uViewProj;
layout(location = 1) uniform float uBaseSize;
layout(location = 4) uniform vec3 uCameraPosition; 

void main()
{
    // particle position in clip space
    gl_Position = uViewProj * vec4(aPosition, 1.0);

    // calculate distance from camera
    float dist = length(aPosition - uCameraPosition);

    // avoids dividng by zero
    dist = max(dist, 0.001);

    // makes the particles smllaer the further they are
    float size = 10.0 * uBaseSize / dist;

    // clamps particles so they dont get too big or too small
    gl_PointSize = clamp(size, 0.0, 400.0);
}
