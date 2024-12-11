#version 300 es
precision highp float;

//ALL CW
vec2 vertices[6] = vec2[](
vec2(-1.0f,  1.0f),
vec2(-1.0f, -1.0f),
vec2(1.0f, -1.0f),

vec2(-1.0f,  1.0f),
vec2(1.0f, -1.0f),
vec2(1.0f,  1.0f)
);

vec2 position_of_the_texture[6] = vec2[](
vec2(0.0f, 1.0f),
vec2(0.0f, 0.0f),
vec2(1.0f, 0.0f),

vec2(0.0f, 1.0f),
vec2(1.0f, 0.0f),
vec2(1.0f, 1.0f)
);

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
    TexCoords = position_of_the_texture[gl_VertexID];
}