#version 330 core
out vec4 frag_color;

in vec4 pos;
in vec3 normal;
in vec2 tex_coord;

uniform mat4 view;

uniform vec4 ambient;
uniform vec4 light_pos;
uniform vec4 light_color;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float time;
uniform float specular_intensity;

void main() {
	vec3 norm = normalize(normal);
	vec3 light_dir = normalize(light_pos.xyz - pos.xyz);

	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(normalize(-pos.xyz), reflect_dir), 0.0), 32);

	vec4 diffuse = light_color * max(dot(norm, light_dir), 0.0);
	vec4 specular = specular_intensity * spec * light_color;

	// mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 0.0) * 
	frag_color = vec4(1.0, 0.5, 0.31, 1.0) * (
		+ ambient
		+ diffuse
		+ specular
	);
}