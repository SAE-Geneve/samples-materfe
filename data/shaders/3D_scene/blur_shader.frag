﻿#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

const float weight[5] = float[](
0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216
);

uniform sampler2D image;
uniform bool horizontal;
void main()
{
    vec2 tex_offset = vec2(1.0) / vec2(textureSize(image, 0)); // Taille d'un texel
    vec3 result = texture(image, TexCoords).rgb * weight[0]; // Contribution centrale

    if (horizontal)
    {
        for (float i = 1.0; i < 5.0; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[int(i)];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[int(i)];
        }
    }
    else
    {
        for (float i = 1.0; i < 5.0; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[int(i)];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[int(i)];
        }
    }
    FragColor = vec4(result, 1.0);
}