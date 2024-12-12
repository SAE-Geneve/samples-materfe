#version 300 es
precision highp float;

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 temp_pos = (projection * view) * vec4(aPos, 1.0);
    gl_Position = temp_pos.xyww;
}