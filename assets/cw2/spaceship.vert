#version 430 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 4) in float iNs; // shininess
layout(location = 5) in vec3 iKa; // ambient
layout(location = 6) in vec3 iKd; // diffuse
layout(location = 7) in vec3 iKe; // emissive
layout(location = 8) in vec3 iKs; // specular

layout(location = 0) uniform mat4 uMvp;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 18) uniform mat4 uModel; // Model matrix for world space

out vec3 vPosition; // world space position for lighting
out vec3 vNormal;
// Material outputs
out vec3 vKa;
out vec3 vKd;
out vec3 vKe;
out vec3 vKs;
out float vNs;

void main()
{
    vec4 worldPosition = uModel * vec4(iPosition, 1.0);
    vPosition = worldPosition.xyz; // Pass world space position to fragment shader
    vNormal = normalize(uNormalMatrix * iNormal);

    // passes material to the fragment shader
    vKa = iKa;
    vKd = iKd;
    vKe = iKe;
    vKs = iKs;
    vNs = iNs;

    gl_Position = uMvp * vec4(iPosition, 1.0);
}
