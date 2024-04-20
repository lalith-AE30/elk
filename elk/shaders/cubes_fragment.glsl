#version 330 core
struct Light {
	vec4 pos;
	
	vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

out vec4 frag_color;

in vec4 pos;
in vec3 normal;
in vec2 tex_coord;

uniform Light light;
uniform Material material;

void main() {
	vec3 norm = normalize(normal);
	vec3 light_dir = normalize(light.pos.xyz - pos.xyz);
	vec3 reflect_dir = reflect(-light_dir, norm);

	float diff = max(dot(norm, light_dir), 0.0);
	float spec = pow(max(dot(normalize(-pos.xyz), reflect_dir), 0.0), material.shininess);

	vec4 ambient  = light.ambient  * (texture(material.diffuse, tex_coord));
	vec4 diffuse  = light.diffuse  * (texture(material.diffuse, tex_coord)  * diff);
	vec4 specular = light.specular * (texture(material.specular, tex_coord) * spec);

	frag_color = ambient + diffuse + specular;
}