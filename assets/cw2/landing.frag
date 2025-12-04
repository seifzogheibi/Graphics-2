#version 430 core

in vec3 vNormal;
in vec3 vColor;

uniform vec3 uLightDir;
uniform vec3 uAmbientColor;

out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    float NdotL = max(dot(N, L), 0.0);

    // Use material colour as base
    vec3 diffuse = NdotL * vColor;
    vec3 ambient = uAmbientColor * vColor;

    vec3 color = ambient + diffuse;
    oColor = vec4(color, 1.0);
}
