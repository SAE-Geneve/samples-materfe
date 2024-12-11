#version 300 es
precision highp float;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 vertices[36] = vec3[](

vec3(-0.5f, -0.5f, -0.5f), //1
vec3(0.5f, -0.5f, -0.5f), //2 -> OK
vec3(0.5f, 0.5f, -0.5f), //3

vec3(0.5f, 0.5f, -0.5f), //3
vec3(-0.5f, 0.5f, -0.5f),  //4 -> OK
vec3(-0.5f, -0.5f, -0.5f), // 1

vec3(0.5f, 0.5f, 0.5f), //7
vec3(0.5f, -0.5f, 0.5f), //6 -> OK
vec3(-0.5f, -0.5f, 0.5f), //5

vec3(-0.5f, -0.5f, 0.5f), //5
vec3(-0.5f, 0.5f, 0.5f), //8 -> OK
vec3(0.5f, 0.5f, 0.5f), //7

vec3(-0.5f, -0.5f, -0.5f), //1
vec3(-0.5f, 0.5f, -0.5f), //4 -> OK
vec3(-0.5f, 0.5f, 0.5f), //8

vec3(-0.5f, 0.5f, 0.5f), //8
vec3(-0.5f, -0.5f, 0.5f), //5 -> OK
vec3(-0.5f, -0.5f, -0.5f), //1

vec3(0.5f, 0.5f, 0.5f), //7
vec3(0.5f, 0.5f, -0.5f), //3 -> OK
vec3(0.5f, -0.5f, -0.5f), //2

vec3(0.5f, -0.5f, -0.5f), //2
vec3(0.5f, -0.5f, 0.5f), //6 -> OK
vec3(0.5f, 0.5f, 0.5f), //7

vec3(0.5f, -0.5f, 0.5f), //6
vec3(0.5f, -0.5f, -0.5f), //2 -> OK
vec3(-0.5f, -0.5f, -0.5f), //1

vec3(-0.5f, -0.5f, -0.5f), //1
vec3(-0.5f, -0.5f, 0.5f), //5 ->OK
vec3(0.5f, -0.5f, 0.5f), //6

vec3(-0.5f, 0.5f, -0.5f), //4
vec3(0.5f, 0.5f, -0.5f), //3 -> OK
vec3(0.5f, 0.5f, 0.5f), //7

vec3(0.5f, 0.5f, 0.5f), //7
vec3(-0.5f, 0.5f, 0.5f), //8 -> OK
vec3(-0.5f, 0.5f, -0.5f) //4
);

void main() {
    gl_Position = projection * view * model * vec4(vertices[gl_VertexID], 1.0);
}
