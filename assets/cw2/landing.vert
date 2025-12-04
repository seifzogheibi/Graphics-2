#version 430 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec3 iColor;  // matches landingMesh colour VBO

layout(location = 2) uniform mat3 uNormalMatrix; // same location idea as terrain
uniform mat4 uProj;                               // this will actually be MVP

out vec3 vNormal;
out vec3 vColor;

void main()
{
    vNormal = normalize(uNormalMatrix * iNormal);
    vColor  = iColor;
    gl_Position = uProj * vec4(iPosition, 1.0);
}
