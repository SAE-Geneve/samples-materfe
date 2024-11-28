#version 300 es
precision highp float;

out vec3 ourColor;
out vec2 texCoord;

vec3 pos[7] = vec3[](
vec3(0.5, -0.5, -0.5),
vec3(0.5, 0.5, -0.5),
vec3(-0.5, 0.5, -0.5),
vec3(-0.5, -0.5, 0.5),
vec3(0.5, -0.5, 0.5),
vec3(0.5, 0.5, 0.5),
vec3(-0.5, 0.5, 0.5)
);

vec3 colors[3] = vec3[](
vec3(1.0, 0.5, 0.7),
vec3(0.0, 1.0, 0.9),
vec3(0.0, 0.3, 0.1)
);

vec2 position_of_the_texture[4] = vec2[](
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),
vec2(0.0, 1.0)
);

void main() {
gl_Position = vec4(pos[gl_VertexID], 1.0);
ourColor = colors[gl_VertexID];
texCoord = position_of_the_texture[gl_VertexID];
}
