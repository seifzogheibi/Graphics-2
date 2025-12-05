// assets/cw2/landing.vert
#version 430 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec3 iColor;      // Kd from MTL (per-vertex)
layout(location = 4) in float iShininess; // Ns from MTL (per-vertex)

layout(location = 0) uniform mat4 uProj;          // MVP
layout(location = 1) uniform mat3 uNormalMatrix;  // normal matrix

out vec3 vNormal;
out vec3 vPosition;
out vec3 vColor;
out float vShininess;

void main()
{
    vNormal    = normalize(uNormalMatrix * iNormal);
    vPosition  = iPosition;
    vColor     = iColor;
    vShininess = iShininess;

    gl_Position = uProj * vec4(iPosition, 1.0);
}
