#version 430

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord; // texture coordinates
layout(location = 2) in vec4 aColor; // per vertex color for buttons outlines and text color
// maps screen space to clip space
layout(location = 0) uniform mat4 uProjection;


out vec2 vTexCoord;
out vec4 vColor;

void main()
{
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
}
