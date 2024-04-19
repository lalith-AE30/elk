#version 330 core
out vec4 frag_color;

in vec3 normal;
in vec2 tex_coord;

uniform vec3 ambient;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float time;


void main() {
	frag_color = vec4(ambient, 1.0) * mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 0.5);
}