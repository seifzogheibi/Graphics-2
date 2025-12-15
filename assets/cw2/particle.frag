#version 430 core

layout(location = 2) uniform vec3 uColor;
layout(location = 3) uniform sampler2D uTexture;
out vec4 outColor;

void main()
{

    // Sample particle texture using point coordinates
    vec4 tex = texture(uTexture, gl_PointCoord);

    // removes very transparent pixels
    if (tex.a < 0.01)
        discard;

    outColor = vec4(tex.rgb * uColor, tex.a); // keep texture alpha
}

