#version 430 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec2 iTexCoord;

layout(location = 0) uniform mat4 uMvp;          // you upload with location 0
layout(location = 1) uniform mat3 uNormalMatrix; // you upload with location 1

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vPosition;   // in world/object space (terrain model is identity)

void main()
{
    vNormal   = normalize(uNormalMatrix * iNormal);
    vTexCoord = iTexCoord;
    vPosition = iPosition;

    gl_Position = uMvp * vec4(iPosition, 1.0);
}
