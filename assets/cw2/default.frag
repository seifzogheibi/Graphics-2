#version 430 core

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vPosition;

// directional light
layout (location = 2) uniform vec3 uLightDir;
layout (location = 3) uniform vec3 uBaseColor;
layout (location = 4) uniform vec3 uAmbientColor;
layout (location = 5) uniform sampler2D uTexture;

// camera and local lights
layout (location = 6) uniform vec3 uCameraPosition;
layout (location = 7) uniform vec3 uLocalLightPosition[3];
layout (location = 10) uniform vec3 uLocalLightColor[3]; 
layout (location = 13) uniform int uLocalLightOn[3];
layout (location = 16) uniform int uDirectionalOn; 
layout (location = 17) uniform int uUseTexture;
out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPosition - vPosition);
    // sample terrain texture
    vec3 texColor = texture(uTexture, vTexCoord).rgb;
    vec3 Kd = texColor * uBaseColor;
    vec3 Ks = vec3(0.05);
    float Ns = 16.0;
    vec3 color = uAmbientColor * Kd;

    // directional light
    if (uDirectionalOn != 0)
    {
        vec3 L = normalize(uLightDir);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);
            vec3 diffuse  = 0.8 * Kd * NdotL;
            vec3 specular = 0.2 * Ks * pow(NdotH, Ns);
            color += diffuse + specular;
        }
    }

    // local point lights with 1/r^2 attenuation
    for (int i = 0; i < 3; ++i)
    {
        if (uLocalLightOn[i] == 0)
            continue;

        vec3 Lvec = uLocalLightPosition[i] - vPosition;
        float dist = length(Lvec);
        vec3 L = Lvec / dist;
        // inverse square law
        float attenuation = 1.0 / max(dist * dist, 1.0);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0)
            continue;

        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);

        vec3 diffuse  = Kd * uLocalLightColor[i] * NdotL;
        vec3 specular = Ks * uLocalLightColor[i] * pow(NdotH, Ns);
        color += attenuation * (diffuse + specular);
    }

    color = min(color, vec3(1.0)); 
    oColor = vec4(color, 1.0);
}
