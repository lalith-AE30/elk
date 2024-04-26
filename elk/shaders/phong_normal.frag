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
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
	sampler2D texture_normal1;
    float shininess;
};

out vec4 frag_color;

in vec4 pos;
in vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;

uniform float time;

#define NR_POINT_LIGHTS 4
uniform PointLight point_lights[NR_POINT_LIGHTS];
uniform SpotLight spot_light;
uniform DirLight dir_light;
uniform Material material;

vec4 calcPointLight(PointLight light, vec4 specular_s, vec4 diffuse_s, vec4 ambient_s, vec4 normal_s) {
	vec4 ray = (view * light.pos) - pos;
	vec4 light_dir = normalize(ray);
	vec4 reflect_dir = reflect(-light_dir, normal_s);

	float diff = max(dot(normal_s, light_dir), 0.0);
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

vec4 calcSpotLight(SpotLight light, vec4 specular_s, vec4 diffuse_s, vec4 ambient_s, vec4 normal_s) {
	vec4 ray = pos - light.pos;
	vec4 light_dir = normalize(ray);
	vec4 reflect_dir = reflect(light_dir, normal_s);

	float diff = max(dot(normal_s, -light_dir), 0.0);
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

vec4 calcDirLight(DirLight light, vec4 specular_s, vec4 diffuse_s, vec4 ambient_s, vec4 normal_s) {
	vec4 light_dir = -normalize(view * light.dir);
	vec4 reflect_dir = reflect(-light_dir, normal_s);

	float diff = max(dot(normal_s, light_dir), 0.0);
	float spec = pow(max(dot(normalize(-pos), reflect_dir), 0.0), 3.0);

	vec4 ambient  = light.ambient  * (ambient_s);
	vec4 diffuse  = light.diffuse  * (diffuse_s  * diff);
	vec4 specular = light.specular * (specular_s * spec);

	return ambient + diffuse + specular;
}

void main() {
	vec4 diffuse_s  =  texture(material.texture_diffuse1, tex_coord);
	if (diffuse_s.a < 0.1) discard;
	vec4 specular_s = texture(material.texture_specular1, tex_coord);
	// TODO Account for non-uniform scaling in future
	vec4 normal_s =  normalize(texture(material.texture_normal1, tex_coord));
	vec4 spot = calcSpotLight(spot_light, specular_s, diffuse_s, diffuse_s, normal_s);
	vec4 dir = calcDirLight(dir_light, specular_s, diffuse_s, diffuse_s, normal_s);
	frag_color = spot;
	for (int i = 0; i < NR_POINT_LIGHTS; i++) {
		frag_color += calcPointLight(point_lights[i], specular_s, diffuse_s, diffuse_s, normal_s);
	}
}