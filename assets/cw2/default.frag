#version 430 core

in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uLightDir;      // direction FROM surface TOWARDS light or vice versa; we’ll be consistent in C++
uniform vec3 uBaseColor;
uniform vec3 uAmbientColor;
uniform sampler2D uTexture;

out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);

    float NdotL = max(dot(N, L), 0.0);

    vec3 texColor = texture(uTexture, vTexCoord).rgb;

    vec3 diffuse = NdotL * uBaseColor * texColor;
    vec3 ambient = uAmbientColor * texColor;

    vec3 color = ambient + diffuse;

    oColor = vec4(color, 1.0);
}
