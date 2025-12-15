#version 430

layout(location = 1) uniform sampler2D uFontTexture; // font atlas texture for drawing text
layout(location = 2) uniform int uUseTexture; // 1 is for text drawing, 0 for solid colors

in vec2 vTexCoord;
in vec4 vColor;

out vec4 FragColor;

void main()
{
    if (uUseTexture == 1)
    {
        // Text rendering
        // font atlas stores text in red
        float alpha = texture(uFontTexture, vTexCoord).r;
        FragColor = vec4(vColor.rgb, vColor.a * alpha);
    }
    else
    {
        // Solid color (buttons, backgrounds, outlines)
        FragColor = vColor;
    }
}
