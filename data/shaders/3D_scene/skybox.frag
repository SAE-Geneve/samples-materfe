#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 Bright;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
    Bright = vec4(0.0);
}