#version 300 es
precision highp float;

//out vec3 ourColor;
out vec2 texCoord;
out vec3 normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 vertices[36] = vec3[](
vec3(-0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, 0.5f, -0.5f),
vec3(0.5f, 0.5f, -0.5f),
vec3(-0.5f, 0.5f, -0.5f),
vec3(-0.5f, -0.5f, -0.5f),

vec3(-0.5f, -0.5f, 0.5f),
vec3(0.5f, -0.5f, 0.5f),
vec3(0.5f, 0.5f, 0.5f),
vec3(0.5f, 0.5f, 0.5f),
vec3(-0.5f, 0.5f, 0.5f),
vec3(-0.5f, -0.5f, 0.5f),

vec3(-0.5f, 0.5f, 0.5f),
vec3(-0.5f, 0.5f, -0.5f),
vec3(-0.5f, -0.5f, -0.5f),
vec3(-0.5f, -0.5f, -0.5f),
vec3(-0.5f, -0.5f, 0.5f),
vec3(-0.5f, 0.5f, 0.5f),

vec3(0.5f, 0.5f, 0.5f),
vec3(0.5f, 0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, 0.5f),
vec3(0.5f, 0.5f, 0.5f),

vec3(-0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, 0.5f),
vec3(0.5f, -0.5f, 0.5f),
vec3(-0.5f, -0.5f, 0.5f),
vec3(-0.5f, -0.5f, -0.5f),

vec3(-0.5f, 0.5f, -0.5f),
vec3(0.5f, 0.5f, -0.5f),
vec3(0.5f, 0.5f, 0.5f),
vec3(0.5f, 0.5f, 0.5f),
vec3(-0.5f, 0.5f, 0.5f),
vec3(-0.5f, 0.5f, -0.5f)
);
//
//vec3 colors[3] = vec3[](
//vec3(1.0, 0.5, 0.7),
//vec3(0.0, 1.0, 0.9),
//vec3(0.0, 0.3, 0.1)
//);

vec2 position_of_the_texture[36] = vec2[](
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0)
);


vec3 aNormal[36] = vec3[]
(
vec3(0.0, 0.0, -1.0),
vec3(0.0f,  0.0f, -1.0f),
vec3(0.0f,  0.0f, -1.0f),
vec3(0.0f,  0.0f, -1.0f),
vec3(0.0f,  0.0f, -1.0f),
vec3(0.0f,  0.0f, -1.0f),
vec3(0.0f,  0.0f, 1.0f),
vec3(0.0f,  0.0f, 1.0f),
vec3(0.0f,  0.0f, 1.0f),
vec3(0.0f,  0.0f, 1.0f),
vec3(0.0f,  0.0f, 1.0f),
vec3(0.0f,  0.0f, 1.0f),
vec3(-1.0f,  0.0f,  0.0f),
vec3(-1.0f,  0.0f,  0.0f),
vec3(-1.0f,  0.0f,  0.0f),
vec3(-1.0f,  0.0f,  0.0f),
vec3(-1.0f,  0.0f,  0.0f),
vec3(-1.0f,  0.0f,  0.0f),
vec3(1.0f,  0.0f,  0.0f),
vec3(1.0f,  0.0f,  0.0f),
vec3(1.0f,  0.0f,  0.0f),
vec3(1.0f,  0.0f,  0.0f),
vec3(1.0f,  0.0f,  0.0f),
vec3(1.0f,  0.0f,  0.0f),
vec3(0.0f, -1.0f,  0.0f),
vec3(0.0f, -1.0f,  0.0f),
vec3(0.0f, -1.0f,  0.0f),
vec3(0.0f, -1.0f,  0.0f),
vec3(0.0f, -1.0f,  0.0f),
vec3(0.0f, -1.0f,  0.0f),
vec3(0.0f,  1.0f,  0.0f),
vec3(0.0f,  1.0f,  0.0f),
vec3(0.0f,  1.0f,  0.0f),
vec3(0.0f,  1.0f,  0.0f),
vec3(0.0f,  1.0f,  0.0f),
vec3(0.0f,  1.0f,  0.0f)
);

void main() {
    gl_Position = projection * view * model * vec4(vertices[gl_VertexID], 1.0);
    //ourColor = colors[gl_VertexID];
    texCoord = position_of_the_texture[gl_VertexID];

    normal = mat3(transpose(inverse(model))) * aNormal[gl_VertexID];
    normal = aNormal[gl_VertexID];
    FragPos = vec3(model * vec4(vertices[gl_VertexID], 1.0));
}
