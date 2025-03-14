#version 330 core
out vec4 frag_color;

in vec3 tex_coord;

uniform samplerCube skybox;

void main() {
    frag_color = texture(skybox, tex_coord);
}