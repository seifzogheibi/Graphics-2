#version 430 core

in vec3 vNormal;

uniform vec3 uLightDir;      // direction FROM surface TOWARDS light or vice versa; we’ll be consistent in C++
uniform vec3 uBaseColor;
uniform vec3 uAmbientColor;

out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = uLightDir;  // if uLightDir is direction FROM light, we negate it here

    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = NdotL * uBaseColor;
    vec3 ambient = uAmbientColor;

    vec3 color = ambient + diffuse;

    oColor = vec4(color, 1.0);
}
