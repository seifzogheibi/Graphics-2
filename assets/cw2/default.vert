#version 430

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;

layout(location = 2) uniform mat3 uNormalMatrix;

layout (location=0) uniform mat4 uProj;

out vec3 vNormal;

void main()
{ 

    // For now assume no non-uniform scaling, so this is fine
    vNormal = normalize(uNormalMatrix * iNormal);

    gl_Position = uProj * vec4( iPosition, 1.0 );
}
    