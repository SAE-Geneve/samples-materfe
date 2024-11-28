#version 300 es
precision highp float;


layout (location = 0) out vec4 outColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    outColor = texture(ourTexture, TexCoord);
}