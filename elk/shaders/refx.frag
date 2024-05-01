#version 330 core

out vec4 frag_color;

in vec4 pos;
in vec4 normal;
in vec2 tex_coord;

uniform samplerCube skybox;

void main() {
    vec3 I = normalize(pos).xyz;
    vec3 N = normalize(normal).xyz;
    vec3 R = refract(I, N, 0.5);
    frag_color = texture(skybox, R);
}