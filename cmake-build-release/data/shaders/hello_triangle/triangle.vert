#version 300 es
precision highp float;


out vec3 ourColor;
out vec2 texCoord;

vec2 positions_for_full_star[3] = vec2[](
vec2(-1.0, -1.0),
vec2(0.0, 1.0),
vec2(1.0, -1.0)
);

vec3 colors[3] = vec3[](
vec3(1.0, 1.0, 1.0),
vec3(1.0, 1.0, 1.0),
vec3(1.0, 1.0, 1.0)
);


vec2 position_of_the_texture[3] = vec2[](
vec2(0.0, 0.0),
vec2(0.5, 1.0),
vec2(1.0, 0.0)
);

void main() {
    gl_Position = vec4(positions_for_full_star[gl_VertexID], 0.0, 1.0);
    ourColor = colors[gl_VertexID];
    texCoord = position_of_the_texture[gl_VertexID];
}
