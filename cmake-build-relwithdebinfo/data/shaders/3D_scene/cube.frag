#version 300 es
precision highp float;

in vec3 ourColor;
in vec2 texCoord;

layout (location = 0) out vec4 outColor;

uniform sampler2D ourTexture1;
//uniform sampler2D ourTexture2;

void main()
{
    vec4 tex1Color = texture(ourTexture1, texCoord);
//    vec4 tex2Color = texture(ourTexture2, texCoord);
//
//    // Example: Blend the two textures (50% each)
//    outColor = mix(tex1Color, tex2Color, 0.5);


    outColor = tex1Color;
}