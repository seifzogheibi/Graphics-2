#version 430 core

// Mesh inputs
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;

layout(location = 4) in float iNs; // shininess
layout(location = 5) in vec3  iKa; // ambient
layout(location = 6) in vec3  iKd; // diffuse
layout(location = 7) in vec3  iKe; // emissive
layout(location = 8) in vec3  iKs; // specular

// Matrices
layout(location = 0) uniform mat4 uProj; // actually viewProj * model (MVP)
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 17) uniform mat4 uModel; // model matrix

out vec3 vNormal;
out vec3 vPosition;       // world space position for lighting
out vec3 vKa;
out vec3 vKd;
out vec3 vKe;
out vec3 vKs;
out float vNs;

void main()
{
    // Puts vertex into world space (for lighitng)
    vec4 worldPosition = uModel * vec4(iPosition, 1.0);
    vPosition = worldPosition.xyz;

    // Rotates the normal into world space
    vNormal = normalize(uNormalMatrix * iNormal);

    vKa = iKa; // goes to fragment shader
    vKd = iKd;
    vKe = iKe;
    vKs = iKs;
    vNs = iNs;

    gl_Position = uProj * worldPosition;  // uProj = viewProj
}
