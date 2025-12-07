#version 430 core

layout(location = 0) out vec4 oColor;

layout(location = 2) uniform vec3 uColor;      // tint
layout(location = 3) uniform sampler2D uTexture;

void main()
{
    // Normalised coords inside the point sprite
    vec2 uv = vec2(gl_PointCoord.x, 1.0 - gl_PointCoord.y);

    // Sample smoke texture (must have alpha)
    vec4 tex = texture(uTexture, uv);

    // Kill almost-transparent pixels so we don't see a square
    if (tex.a < 0.01)
        discard;

    // Standard alpha blending: color from texture * tint, alpha from texture
    vec3 rgb   = uColor * tex.rgb;
    float alpha = tex.a;

    oColor = vec4(rgb, alpha);
}
