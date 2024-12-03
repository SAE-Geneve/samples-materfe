#version 300 es
precision highp float;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 vertices[36] = vec3[](
vec3(-0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f,  0.5f, -0.5f),
vec3(0.5f,  0.5f, -0.5f),
vec3(-0.5f,  0.5f, -0.5f),
vec3(-0.5f, -0.5f, -0.5f),

vec3(-0.5f, -0.5f,  0.5f),
vec3(0.5f, -0.5f,  0.5f),
vec3(0.5f,  0.5f,  0.5f),
vec3(0.5f,  0.5f,  0.5f),
vec3(-0.5f,  0.5f,  0.5f),
vec3(-0.5f, -0.5f,  0.5f),

vec3(-0.5f,  0.5f,  0.5f),
vec3(-0.5f,  0.5f, -0.5f),
vec3(-0.5f, -0.5f, -0.5f),
vec3(-0.5f, -0.5f, -0.5f),
vec3(-0.5f, -0.5f,  0.5f),
vec3(-0.5f,  0.5f,  0.5f),

vec3(0.5f,  0.5f,  0.5f),
vec3(0.5f,  0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f,  0.5f),
vec3(0.5f,  0.5f,  0.5f),

vec3(-0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f, -0.5f),
vec3(0.5f, -0.5f,  0.5f),
vec3(0.5f, -0.5f,  0.5f),
vec3(-0.5f, -0.5f,  0.5f),
vec3(-0.5f, -0.5f, -0.5f),

vec3(-0.5f,  0.5f, -0.5f),
vec3(0.5f,  0.5f, -0.5f),
vec3(0.5f,  0.5f,  0.5f),
vec3(0.5f,  0.5f,  0.5f),
vec3(-0.5f,  0.5f,  0.5f),
vec3(-0.5f,  0.5f, -0.5f)
);

void main() {
    gl_Position = projection * view * model * vec4(vertices[gl_VertexID], 1.0);
}
