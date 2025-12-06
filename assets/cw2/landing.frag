// assets/cw2/landing.frag
#version 430 core

in vec3 vNormal;
in vec3 vPosition;

// Material data from landing.vert / SimpleMeshData
in vec3 vKa;          // ambient   (Ka)
in vec3 vKd;          // diffuse   (Kd)
in vec3 vKe;          // emissive  (Ke)
in vec3 vKs;          // specular  (Ks)
in float vNs;  // shininess (Ns)

// keep same layout indices as terrain
layout (location = 2)  uniform vec3 uLightDir;
layout (location = 4)  uniform vec3 uAmbientColor;

layout (location = 6)  uniform vec3 uCameraPos;
layout (location = 7)  uniform vec3 uPointLightPos[3];      // 7, 8, 9
layout (location = 10) uniform vec3 uPointLightColor[3];    // 10, 11, 12
layout (location = 13) uniform int  uPointLightEnabled[3];  // 13, 14, 15
layout (location = 16) uniform int  uDirectionalEnabled;    // 16

out vec4 oColor;

// Make them brighter than terrain but not insane
const float POINT_INTENSITY = 1.0;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vPosition);

    // Material parameters from .mtl
    vec3 Ka = vKa;
    vec3 Kd = vKd;
    vec3 Ks = vKs;
    float Ns = max(vNs, 1.0);   // avoid zero / NaN

    // Softer ambient, small emissive
    vec3 color = 0.3 * Ka * uAmbientColor + 0.3 * vKe;

    // ------------------------
    // Directional light
    // ------------------------
    if (uDirectionalEnabled != 0)
    {
        vec3 L = normalize(uLightDir);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);

            // keep this fairly subtle – main show is point lights
            vec3 diffuse  = 0.4 * Kd * NdotL;
            vec3 specular = 0.2 * Ks * pow(NdotH, Ns);

            color += diffuse + specular;
        }
    }

    // ------------------------
    // Point lights (Blinn–Phong, 1/r^2 attenuation)
    // ------------------------
    for (int i = 0; i < 3; ++i)
    {
        if (uPointLightEnabled[i] == 0)
            continue;

        vec3 Lvec = uPointLightPos[i] - vPosition;
        float dist = length(Lvec);
        vec3 L = Lvec / max(dist, 0.0001);

        // REQUIRED: 1/r^2 attenuation
        float attenuation = 1.0 / max(dist * dist, 0.0001);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0)
            continue;

        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);

        // brighter point lights, but still controlled
        vec3 lightColor = uPointLightColor[i] * POINT_INTENSITY;

        // more “reflection-y”: diffuse smaller, specular larger
        vec3 diffuse  = Kd * lightColor * NdotL * 1.0 / 3.141592;
        vec3 specular = Ks * lightColor * pow(NdotH, Ns) * (Ns + 2.0) / 8.0;

        color += attenuation * (diffuse + specular);
    }

    // Avoid ugly clipping / banding
    color = min(color, vec3(1.0));

    oColor = vec4(color, 1.0);
}
