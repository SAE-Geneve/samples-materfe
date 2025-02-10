#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool reverse;
uniform bool reverseGammaEffect;

void main()
{
    //vec4 col = texture(screenTexture, TexCoords);
    if(reverse)
    {
        FragColor = vec4(1.0 - texture(screenTexture, TexCoords));
    }
    else if (reverseGammaEffect)
    {
        float gamma = 2.2;
        vec3 finalColor = vec3(texture(screenTexture, TexCoords));
        finalColor = pow(finalColor, vec3(gamma));
        FragColor = vec4(finalColor, 1.0);
    }
    else
    {
        FragColor = vec4(texture(screenTexture, TexCoords));
    }
    //for inverse -> 1.0 - text
}
