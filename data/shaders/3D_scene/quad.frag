#version 300 es
precision highp float;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool reverse;

void main()
{
    //vec4 col = texture(screenTexture, TexCoords);
    if(reverse)
    {
        FragColor = vec4(1.0 - texture(screenTexture, TexCoords));
    }
    else
    {
        FragColor = vec4(texture(screenTexture, TexCoords));
    }
    //for inverse -> 1.0 - text
}
