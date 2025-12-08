#version 430 core

layout(location = 2) uniform sampler2D uParticleTex;
layout(location = 3) uniform vec3 uParticleColor;

out vec4 FragColor;

void main()
{
<<<<<<< HEAD
    // For point sprites, gl_PointCoord gives [0,1] UV inside the sprite
    vec2 uv   = gl_PointCoord;
    vec4 tex  = texture(uParticleTex, uv);

    // Multiply texture (with alpha) by chosen colour
    FragColor = vec4(uParticleColor, 1.0) * tex;

    // If your texture has soft edges/alpha, its alpha will control blending
=======
    // Normalised coords inside the point sprite
    vec2 uv = vec2(gl_PointCoord.x, gl_PointCoord.y);

    // Sample smoke texture (must have alpha)
    vec4 tex = texture(uTexture, gl_PointCoord);

    // Kill almost-transparent pixels so we don't see a square
    if (tex.a < 0.01)
        discard;

    // Standard alpha blending: color from texture * tint, alpha from texture
    float alpha = tex.a;

    oColor = vec4(tex.rgb, alpha);
>>>>>>> 7f5b858 (updated)
}
