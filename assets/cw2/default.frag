#version 430 core

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vPosition;

// existing ones
layout (location = 2) uniform vec3 uLightDir;
layout (location = 3) uniform vec3 uBaseColor;
layout (location = 4) uniform vec3 uAmbientColor;
layout (location = 5) uniform sampler2D uTexture;

// NEW – fixed locations, no glGetUniformLocation
layout (location = 6) uniform vec3 uCameraPos;
layout (location = 7) uniform vec3 uPointLightPos[3];
layout (location = 8) uniform vec3 uPointLightColor[3];
layout (location = 9) uniform int  uPointLightEnabled[3];
layout (location = 10) uniform int uDirectionalEnabled;

out vec4 oColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vPosition);

    // Terrain material
    vec3 texColor = texture(uTexture, vTexCoord).rgb;
    vec3 Kd = texColor * uBaseColor;  // diffuse
    vec3 Ks = vec3(0.3);              // simple specular
    float shininess = 32.0;

    // Ambient
    vec3 ambient = uAmbientColor * Kd;
    vec3 color   = ambient;

    // ---------- Directional light ----------
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

    // ---------- 3 point lights with 1/r^2 ----------
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
