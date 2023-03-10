
#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;
layout (location = 2) in vec2 vertex_texcoord;
layout (location = 3) in vec3 vertex_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 clipSpace;

void main()
{
    gl_Position = projection * view * model * vec4(vertex_position, 1.0);
    clipSpace = gl_Position;
}