#version 430 core

// Matches the locations used in main.cpp
layout(location = 2) uniform vec3 uColor;        // tint for the exhaust
layout(location = 3) uniform sampler2D uTexture; // particle texture

out vec4 outColor;

void main()
{

    // Sample smoke/exhaust texture (must have alpha)
    vec4 tex = texture(uTexture, gl_PointCoord);

    // Kill almost-transparent pixels so we don't see a square
    if (tex.a < 0.01)
        discard;

    // Tint the RGB by uColor, keep alpha from texture
    outColor = vec4(tex.rgb * uColor, tex.a);
}

