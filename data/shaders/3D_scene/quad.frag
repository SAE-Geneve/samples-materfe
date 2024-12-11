#version 300 es
precision highp float;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    //vec4 col = texture(screenTexture, TexCoords);
    FragColor = vec4(texture(screenTexture, TexCoords));
}