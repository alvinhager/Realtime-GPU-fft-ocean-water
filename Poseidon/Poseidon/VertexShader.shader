#version 450 core

in layout(location = 0) vec3 in_position;
in layout(location = 1) vec2 in_texcoord;

out vec2 texcoord;

void main() {
    gl_Position = vec4(in_position, 1);
    texcoord = in_texcoord;
}