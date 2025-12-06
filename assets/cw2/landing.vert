#version 430 core

// Attributes from SimpleMeshData / create_vao (non-textured path)
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec3 iColor;     // not strictly needed, but OK to keep

layout(location = 4) in float iShininess;
layout(location = 5) in vec3  iKa;       // ambient
layout(location = 6) in vec3  iKd;       // diffuse
layout(location = 7) in vec3  iKe;       // emissive
layout(location = 8) in vec3  iKs;       // specular

// Matrices
layout(location = 0) uniform mat4 uProj;          // actually viewProj * model (MVP)
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 17) uniform mat4 uModel;        // NEW: pure model matrix

out vec3 vNormal;
out vec3 vPosition;       // world-space position
out vec3 vKa;
out vec3 vKd;
out vec3 vKe;
out vec3 vKs;
out float vShininess;

void main()
{
    // World-space position
    vec4 worldPos = uModel * vec4(iPosition, 1.0);
    vPosition     = worldPos.xyz;

    // Normal (no non-uniform scaling, so uNormalMatrix is fine)
    vNormal = normalize(uNormalMatrix * iNormal);

    vKa        = iKa;
    vKd        = iKd;
    vKe        = iKe;
    vKs        = iKs;
    vShininess = iShininess;

    gl_Position = uProj * worldPos;  // uProj = viewProj
}
