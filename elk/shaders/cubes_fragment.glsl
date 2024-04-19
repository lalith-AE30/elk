#version 330 core
struct Light {
	vec4 pos;
	
	vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

out vec4 frag_color;

in vec4 pos;
in vec3 normal;
in vec2 tex_coord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform Light light;
uniform Material material;

void main() {
	vec3 norm = normalize(normal);
	vec3 light_dir = normalize(light.pos.xyz - pos.xyz);

	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(normalize(-pos.xyz), reflect_dir), 0.0), material.shininess);

	float diff = max(dot(norm, light_dir), 0.0);

	vec4 ambient  = light.ambient * (material.ambient);
	vec4 diffuse  = light.diffuse * (diff * material.diffuse);
	vec4 specular = light.specular * (spec * material.specular);

	// mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 0.0) * 
	frag_color = ambient + diffuse + specular;
}