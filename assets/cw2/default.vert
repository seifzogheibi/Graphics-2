#version 430

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec2 iTexCoord;

layout(location = 0) uniform mat4 uProj;
layout(location = 1) uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vPosition; 

void main()
{ 

    // For now assume no non-uniform scaling, so this is fine
    vNormal = normalize(uNormalMatrix * iNormal);
    vTexCoord = iTexCoord;
    vPosition = iPosition;

    gl_Position = uProj * vec4( iPosition, 1.0 );
}
    