#version 430

layout(location = 1) uniform sampler2D uFontTexture;
layout(location = 2) uniform int uUseTexture;

in vec2 vTexCoord;
in vec4 vColor;

out vec4 FragColor;

void main()
{
    if (uUseTexture == 1)
    {
        // Text rendering - use font atlas
        float alpha = texture(uFontTexture, vTexCoord).r;
        FragColor = vec4(vColor.rgb, vColor.a * alpha);
    }
    else
    {
        // Solid color (buttons, backgrounds)
        FragColor = vColor;
    }
}