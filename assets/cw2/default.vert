#version 430 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
// layout(location = 2) in vec3 iColor;     // Not used here
layout(location = 3) in vec2 iTexCoord;  // For terrain (textured)
// Material properties (for UFO with materials)
layout(location = 4) in float iNs;
layout(location = 5) in vec3  iKa;       // ambient
layout(location = 6) in vec3  iKd;       // diffuse
layout(location = 7) in vec3  iKe;       // emissive
layout(location = 8) in vec3  iKs;       // specular

layout(location = 0) uniform mat4 uMvp;          // you upload with location 0
layout(location = 1) uniform mat3 uNormalMatrix; // you upload with location 1
layout(location = 18) uniform mat4 uModel;       // Model matrix for world space

out vec3 vPosition;   // world space position for lighting
out vec3 vNormal;
// out vec3 vColor;
out vec2 vTexCoord;
// Material outputs
out vec3 vKa;
out vec3 vKd;
out vec3 vKe;
out vec3 vKs;
out float vNs;

void main()
{
    // Transform to world space (for terrain, model is identity, so this is essentially iPosition)
    vec4 worldPos = uModel * vec4(iPosition, 1.0);
    
    vPosition = worldPos.xyz;   // Pass world space position to fragment shader
    vNormal   = normalize(uNormalMatrix * iNormal);
    // vColor = iColor;
    vTexCoord = iTexCoord;

    
    // Pass through material properties
    vKa = iKa;
    vKd = iKd;
    vKe = iKe;
    vKs = iKs;
    vNs = iNs;

    gl_Position = uMvp * vec4(iPosition, 1.0);
}
