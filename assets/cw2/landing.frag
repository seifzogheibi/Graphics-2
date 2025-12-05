// assets/cw2/landing.frag
#version 430 core

in vec3 vNormal;
in vec3 vPosition;
in vec3 vColor;      // Kd
in vec3 vAmbient; // Ka
in vec3 vSpecular; // Ks
in vec3 vEmissive; // Ke

in float vShininess; // Ns

// keep same layout indices as terrain
layout (location = 2) uniform vec3 uLightDir;
layout (location = 4) uniform vec3 uAmbientColor;

layout (location = 6) uniform vec3 uCameraPos;
layout (location = 7) uniform vec3 uPointLightPos[3];      // uses 7, 8, 9
layout (location = 10) uniform vec3 uPointLightColor[3];   // uses 10, 11, 12
layout (location = 13) uniform int  uPointLightEnabled[3]; // uses 13, 14, 15
layout (location = 16) uniform int uDirectionalEnabled;    // uses 16

out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vPosition);

    vec3 Kd = vColor;
    vec3 Ks = vSpecular;        // specular from material
    float shininess = vShininess;

    vec3 ambient = uAmbientColor * Kd + vEmissive;
    vec3 color   = ambient;

    // Directional
    if (uDirectionalEnabled != 0)
    {
        vec3 L = normalize(-uLightDir);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);

            vec3 diffuse  = Kd * NdotL;
            vec3 specular = Ks * pow(NdotH, shininess);

            color += diffuse + specular;
        }
    }

    // Point lights
    for (int i = 0; i < 3; ++i)
    {
        if (uPointLightEnabled[i] == 0)
            continue;

        vec3 Lvec = uPointLightPos[i] - vPosition;
        float dist = length(Lvec);
        vec3 L = Lvec / max(dist, 0.0001);

        float attenuation = 1.0 / (dist * dist);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0)
            continue;

        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);

        vec3 diffuse  = Kd * uPointLightColor[i] * NdotL;
        vec3 specular = Ks * uPointLightColor[i] * pow(NdotH, shininess);

        color += attenuation * (diffuse + specular);
    }

    oColor = vec4(color, 1.0);
}
