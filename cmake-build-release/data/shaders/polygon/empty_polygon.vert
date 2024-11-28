#version 310 es
precision highp float;

out vec3 fragColor;

vec2 positions_for_empty_polygon[6] = vec2[](
vec2(0.0, 1.0),
vec2(1.0, 0.5),
vec2(0.5, -0.9),
vec2(-0.5, -0.9),
vec2(-1.0, 0.5),
vec2(0.0,1.0)
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
    gl_Position = vec4(positions_for_empty_polygon[gl_VertexID], 0.0, 1.0);
    fragColor = colors[gl_VertexID];
}
