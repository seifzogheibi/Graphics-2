#version 430 core

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vPosition;

// existing ones
layout (location = 2) uniform vec3 uLightDir;
layout (location = 3) uniform vec3 uBaseColor;
layout (location = 4) uniform vec3 uAmbientColor;
layout (location = 5) uniform sampler2D uTexture;

// NEW – fixed locations
layout (location = 6) uniform vec3 uCameraPos;
layout (location = 7) uniform vec3 uPointLightPos[3];
layout (location = 10) uniform vec3 uPointLightColor[3];
layout (location = 13) uniform int  uPointLightEnabled[3];
layout (location = 16) uniform int  uDirectionalEnabled;

out vec4 oColor;

void main()
{
    // First just check the texture again
    vec3 texColor = texture(uTexture, vTexCoord).rgb;
    oColor = vec4(texColor, 1.0);
}
