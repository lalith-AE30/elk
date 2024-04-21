#version 330 core
struct PointLight {
	vec4 pos;
	
	vec4 ambient;
    vec4 diffuse;
    vec4 specular;

	vec3 visibility;
};

struct SpotLight {
	vec4 pos;
	vec4 dir;
	float soft_cutoff;
	float cutoff;
	
	vec4 ambient;
    vec4 diffuse;
    vec4 specular;

	vec3 visibility;
};

struct DirLight {
	vec4 dir;
	
	vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
	sampler2D emission;
    float shininess;
};

out vec4 frag_color;

in vec4 pos;
in vec4 normal;
in vec2 tex_coord;

uniform mat4 view;

uniform float time;

#define NR_POINT_LIGHTS 4
uniform PointLight point_lights[NR_POINT_LIGHTS];
uniform SpotLight spot_light;
uniform DirLight dir_light;
uniform Material material;

vec4 norm = normalize(normal);
vec4 specular_s = texture(material.specular, tex_coord);
vec4 diffuse_s = texture(material.diffuse, tex_coord);
vec4 ambient_s = diffuse_s;

vec4 calcPointLight(PointLight light) {
	vec4 ray = (view * light.pos) - pos;
	vec4 light_dir = normalize(ray);
	vec4 reflect_dir = reflect(-light_dir, norm);

	float diff = max(dot(norm, light_dir), 0.0);
	float spec = pow(max(dot(normalize(-pos), reflect_dir), 0.0), material.shininess);

	vec4 ambient  = light.ambient  * (ambient_s);
	vec4 diffuse  = light.diffuse  * (diffuse_s  * diff);
	vec4 specular = light.specular * (specular_s * spec);

	float attenuation = 1.0 / (
		light.visibility.x + 
		light.visibility.y * length(ray) + 
		light.visibility.z * length(ray) * length(ray)
	);

	return (ambient + diffuse + specular) * attenuation;
}

vec4 calcSpotLight(SpotLight light) {
	vec4 ray = pos - light.pos;
	vec4 light_dir = normalize(ray);
	vec4 reflect_dir = reflect(light_dir, norm);

	float diff = max(dot(norm, -light_dir), 0.0);
	float spec = pow(max(dot(normalize(-pos), reflect_dir), 0.0), material.shininess);

	vec4 ambient  = light.ambient  * (ambient_s);
	vec4 diffuse  = light.diffuse  * (diffuse_s  * diff);
	vec4 specular = light.specular * (specular_s * spec);

	float attenuation = 1.0 / (
		light.visibility.x + 
		light.visibility.y * length(ray) + 
		light.visibility.z * length(ray) * length(ray)
	);

	float val = dot(light_dir, light.dir);
	float I = clamp((val-light.cutoff)/(light.soft_cutoff-light.cutoff), 0.0, 1.0);

	return (ambient + diffuse + specular) * attenuation * I;
};

vec4 calcDirLight(DirLight light) {
	vec4 light_dir = -normalize(view * light.dir);
	vec4 reflect_dir = reflect(-light_dir, norm);

	float diff = max(dot(norm, light_dir), 0.0);
	float spec = pow(max(dot(normalize(-pos), reflect_dir), 0.0), material.shininess);

	vec4 ambient  = light.ambient  * (ambient_s);
	vec4 diffuse  = light.diffuse  * (diffuse_s  * diff);
	vec4 specular = light.specular * (specular_s * spec);

	return ambient + diffuse + specular;
}

void main() {
	vec4 emission = texture(material.emission, tex_coord);
	vec4 spot = calcSpotLight(spot_light);
	vec4 dir = calcDirLight(dir_light);
	frag_color = spot + emission;
	for (int i = 0; i < NR_POINT_LIGHTS; i++) {
		frag_color += calcPointLight(point_lights[i]);
	}
}