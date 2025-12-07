#version 430 core

layout(location = 2) uniform sampler2D uParticleTex;
layout(location = 3) uniform vec3 uParticleColor;

out vec4 FragColor;

void main()
{
    // For point sprites, gl_PointCoord gives [0,1] UV inside the sprite
    vec2 uv   = gl_PointCoord;
    vec4 tex  = texture(uParticleTex, uv);

    // Multiply texture (with alpha) by chosen colour
    FragColor = vec4(uParticleColor, 1.0) * tex;

    // If your texture has soft edges/alpha, its alpha will control blending
}
