// assets/cw2/landing.frag
//#version 430 core
// Mac version
#version 410 core

in vec3 vNormal;
in vec3 vPosition;

// Material data from landing.vert / SimpleMeshData
in vec3 vKa;
in vec3 vKd;
in vec3 vKe;
in vec3 vKs;
in float vNs;

// keeps same layout as terrain shader
//layout (location = 2)  uniform vec3 uLightDir;
//layout (location = 4)  uniform vec3 uAmbientColor;
//
//layout (location = 6)  uniform vec3 uCameraPosition;
//// spaceship lights
//layout (location = 7)  uniform vec3 uLocalLightPosition[3];
//layout (location = 10) uniform vec3 uLocalLightColor[3];
//layout (location = 13) uniform int  uLocalLightOn[3];
//layout (location = 16) uniform int  uDirectionalOn; // directional light

uniform vec3 uLightDir;
uniform vec3 uAmbientColor;

uniform vec3 uCameraPosition;
// spaceship lights
uniform vec3 uLocalLightPosition[3];
uniform vec3 uLocalLightColor[3];
uniform int  uLocalLightOn[3];
uniform int  uDirectionalOn; // directional light

out vec4 oColor;

// Make them brighter than terrain
const float LocalLightBrightness = 1.0;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPosition - vPosition);
    vec3 Ka = vKa;
    vec3 Kd = vKd;
    vec3 Ks = vKs;
    float Ns = max(vNs, 1.0);   // avoid zero

    // Softer ambient and small emissive
    vec3 color = 0.3 * Ka * uAmbientColor + 0.3 * vKe;

    // Directional light
    if (uDirectionalOn != 0)
    {
        vec3 L = normalize(uLightDir);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            // Blinn–Phong
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);

            vec3 diffuse  = 0.4 * Kd * NdotL;
            vec3 specular = 0.2 * Ks * pow(NdotH, Ns);
            color += diffuse + specular;
        }
    }

    // Point lights (Blinn–Phong with 1/r^2 falloff)
    for (int i = 0; i < 3; ++i)
    {
        if (uLocalLightOn[i] == 0)
            continue;

        vec3 Lvec = uLocalLightPosition[i] - vPosition;
        float dist = length(Lvec);
        vec3 L = Lvec / max(dist, 0.0001);

        // 1/r^2 attenuation its brighter upclose and fades fast
        float attenuation = 1.0 / max(dist * dist, 0.0001);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0)
            continue;

        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);
        vec3 lightColor = uLocalLightColor[i] * LocalLightBrightness;

        // makes it more shiny by reducing diffuse and making specular stronger
        vec3 diffuse  = Kd * lightColor * NdotL * 1.0 / 3.141592;
        vec3 specular = Ks * lightColor * pow(NdotH, Ns) * (Ns + 2.0) / 8.0;
        color += attenuation * (diffuse + specular);
    }

    color = min(color, vec3(1.0));
    oColor = vec4(color, 1.0);
}
