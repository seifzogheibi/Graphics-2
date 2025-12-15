//#version 430 core
// Mac version
#version 410 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 3) in vec2 iTexCoord;  // For terrain (textured)
//// Material properties (for UFO with materials)
//layout(location = 4) in float iNs;
//layout(location = 5) in vec3  iKa;       // ambient
//layout(location = 6) in vec3  iKd;       // diffuse
//layout(location = 7) in vec3  iKe;       // emissive
//layout(location = 8) in vec3  iKs;       // specular

//layout(location = 0) uniform mat4 uMvp;          // you upload with location 0
//layout(location = 1) uniform mat3 uNormalMatrix; // you upload with location 1
//layout(location = 18) uniform mat4 uModel;       // Model matrix for world space
uniform mat4 uMvp;
uniform mat3 uNormalMatrix;
uniform mat4 uModel;

out vec3 vPosition;   // world space position for lighting
out vec3 vNormal;
// out vec3 vColor;
out vec2 vTexCoord;

void main()
{
    // Transform to world space (for terrain, model is identity, so this is essentially iPosition)
    vec4 worldPosition = uModel * vec4(iPosition, 1.0);
    
    vPosition = worldPosition.xyz;   // Pass world space position to fragment shader
    vNormal = normalize(uNormalMatrix * iNormal);
    vTexCoord = iTexCoord;

    gl_Position = uMvp * vec4(iPosition, 1.0);
}
