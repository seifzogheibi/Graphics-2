#version 430 core

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vPosition;
// Material properties from vertex shader
in vec3 vKa;
in vec3 vKd;
in vec3 vKe;
in vec3 vKs;
in float vNs;

// Directional light + material base colour
layout (location = 2) uniform vec3 uLightDir;
layout (location = 3) uniform vec3 uBaseColor;
layout (location = 4) uniform vec3 uAmbientColor;
layout (location = 5) uniform sampler2D uTexture;

// Camera + point lights
layout (location = 6)  uniform vec3 uCameraPos;
layout (location = 7)  uniform vec3 uPointLightPos[3];      // 7, 8, 9
layout (location = 10) uniform vec3 uPointLightColor[3];    // 10, 11, 12
layout (location = 13) uniform int  uPointLightEnabled[3];  // 13, 14, 15
layout (location = 16) uniform int  uDirectionalEnabled;    // 16
layout (location = 17) uniform int uUseTexture;  // 0 = use materials, 1 = use texture


out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vPosition);


   // --- Material parameters ---
    vec3 Ka, Kd, Ks, Ke;
    float Ns;

    
    if (uUseTexture != 0) {
        // Terrain mode: use texture
        vec3 texColor = texture(uTexture, vTexCoord).rgb;
        Ka = uAmbientColor;
        Kd = texColor * uBaseColor;
        Ks = vec3(0.05);
        Ke = vec3(0.0);
        Ns = 16.0;
    } else {
        // UFO mode: use per-vertex material properties
        Ka = vKa;
        Kd = vKd;
        Ks = vKs;
        Ke = vKe;
        Ns = max(vNs, 1.0);
    }

    // Ambient term
    vec3 color = uAmbientColor * Kd;

    // ----- Directional light -----
    if (uDirectionalEnabled != 0)
    {
        // uLightDir is the direction the light TRAVELS (from light to scene)
        // For Lambert we want vector FROM point TO light => -uLightDir
        vec3 L = normalize(uLightDir);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);

            // Slightly reduce influence so it doesn’t blow out
            vec3 diffuse  = 0.8 * Kd * NdotL;
            vec3 specular = 0.2 * Ks * pow(NdotH, Ns);

            color += diffuse + specular;
        }
    }

    // ----- 3 point lights with 1/r^2 attenuation -----
    for (int i = 0; i < 3; ++i)
    {
        if (uPointLightEnabled[i] == 0)
            continue;

        vec3 Lvec = uPointLightPos[i] - vPosition;
        float dist = length(Lvec);
        // distc = 1.0;
        // vec3 L = Lvec / max(dist, 0.0001);
        vec3 L = Lvec / dist;

        float attenuation = 1.0 / max(dist * dist, 1.0);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0)
            continue;

        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);

        vec3 diffuse  = Kd * uPointLightColor[i] * NdotL;
        vec3 specular = Ks * uPointLightColor[i] * pow(NdotH, Ns);

        color += attenuation * (diffuse + specular);
    }

    // Avoid over-bright white “burnout”
    color = min(color, vec3(1.0));

    oColor = vec4(color, 1.0);
}
