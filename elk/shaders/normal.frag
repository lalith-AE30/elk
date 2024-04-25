#version 330 core
out vec4 frag_color;

in vec4 normal;

void main() {
    frag_color = normalize(normal);
}