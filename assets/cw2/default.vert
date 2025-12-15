#version 430 core
// mesh inputs
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec2 iTexCoord; // terrain texture coordinates

layout(location = 0) uniform mat4 uMvp; // modelview projection matrix
layout(location = 1) uniform mat3 uNormalMatrix; // for transforming normals
layout(location = 18) uniform mat4 uModel; // model matrix for world space

out vec3 vPosition; // world space position for lighting
out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
    // transforms vertex position to world space
    vec4 worldPosition = uModel * vec4(iPosition, 1.0);
    vPosition = worldPosition.xyz;
    vNormal = normalize(uNormalMatrix * iNormal); // transform normal to world space
    vTexCoord = iTexCoord;
    gl_Position = uMvp * vec4(iPosition, 1.0);
}
