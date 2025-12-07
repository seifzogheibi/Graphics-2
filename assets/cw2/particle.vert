#version 430 core

layout(location = 0) in vec3 aPosition;

layout(location = 0) uniform mat4 uViewProj;
layout(location = 1) uniform float uPointSize;

void main()
{
    gl_Position  = uViewProj * vec4(aPosition, 1.0);
    gl_PointSize = uPointSize;
}