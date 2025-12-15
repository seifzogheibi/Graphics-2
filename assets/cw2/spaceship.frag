#version 430 core

in vec3 vNormal;
in vec3 vPosition;

in vec3 vKa;
in vec3 vKd;
in vec3 vKe;
in vec3 vKs;
in float vNs;

layout (location = 2)  uniform vec3 uLightDir;
layout (location = 4)  uniform vec3 uAmbientColor;

layout (location = 6)  uniform vec3 uCameraPosition;
layout (location = 7)  uniform vec3 uLocalLightPosition[3];   // 7,8,9
layout (location = 10) uniform vec3 uLocalLightColor[3];      // 10,11,12
layout (location = 13) uniform int  uLocalLightOn[3];         // 13,14,15
layout (location = 16) uniform int  uDirectionalOn;           // 16

out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPosition - vPosition);

    vec3 Ka = vKa;
    vec3 Kd = vKd;
    vec3 Ks = vKs;
    vec3 Ke = vKe;
    float Ns = max(vNs, 1.0);

    // keep EXACT same “look” logic you currently use in UFO mode:
    vec3 color = uAmbientColor * Kd;

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

    for (int i = 0; i < 3; ++i)
    {
        if (uLocalLightOn[i] == 0) continue;

        vec3 Lvec = uLocalLightPosition[i] - vPosition;
        float dist = length(Lvec);
        vec3 L = Lvec / dist;

        float attenuation = 1.0 / max(dist * dist, 1.0);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);

        vec3 diffuse  = Kd * uLocalLightColor[i] * NdotL;
        vec3 specular = Ks * uLocalLightColor[i] * pow(NdotH, Ns);

        color += attenuation * (diffuse + specular);
    }

    color = min(color, vec3(1.0));
    oColor = vec4(color, 1.0);
}
