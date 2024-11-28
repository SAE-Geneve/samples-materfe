#version 310 es
precision highp float;

out vec3 fragColor;

vec2 positions_for_full_quad[6] = vec2[](
vec2(-0.9, 0.9),
vec2(0.9, 0.9),
vec2(0.9, -0.9),
vec2(0.9, -0.9),
vec2(-0.9, -0.9),
vec2(-0.9, 0.9)
);

vec3 colors[6] = vec3[](
vec3(1.0, 1.0, 0.0),
vec3(1.0, 1.0, 0.0),
vec3(1.0, 1.0, 0.0),
vec3(1.0, 1.0, 0.0),
vec3(1.0, 1.0, 0.0),
vec3(1.0, 1.0, 0.0)
);

void main() {
    gl_Position = vec4(positions_for_full_quad[gl_VertexID], 0.0, 1.0);
    fragColor = colors[gl_VertexID];
}