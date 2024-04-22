#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <vector>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

struct Material {
    GLuint diffuse;
    GLuint specular;
    GLuint emission;
    float shininess;
};

struct DirectionalLight {
    glm::vec4 dir;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

struct PointLight {
    glm::vec4 pos;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    glm::vec3 visibility;
};

struct SpotLight {
    glm::vec4 pos;
    glm::vec4 dir;
    float soft_cutoff;
    float cutoff;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    glm::vec3 visibility;
};

void setVisibility(PointLight& light, float distance) {
    light.visibility = glm::vec3(1.0f, 4.5f / distance, 75.0f / (distance * distance));
}
void setVisibility(SpotLight& light, float distance) {
    light.visibility = glm::vec3(1.0f, 4.5f / distance, 75.0f / (distance * distance));
}

void updateMaterialShader(Shader& shader, SpotLight& light, float shininess, float time, bool disable_emission = false) {
    shader.setVec4("spot_light.pos", light.pos);
    shader.setVec4("spot_light.dir", light.dir);
    shader.setFloat("spot_light.soft_cutoff", light.soft_cutoff);
    shader.setFloat("spot_light.cutoff", light.cutoff);
    shader.setVec4("spot_light.ambient", light.ambient);
    shader.setVec4("spot_light.diffuse", light.diffuse);
    shader.setVec4("spot_light.specular", light.specular);

    shader.setVec3("spot_light.visibility", light.visibility);

    shader.setFloat("time", time);

    shader.setFloat("material.shininess", shininess);
}
void updateMaterialShader(
    Shader& shader,
    SpotLight& spot_light,
    std::vector<PointLight>& point_lights,
    DirectionalLight& dir_light,
    float shininess = 32.0f,
    float time = 0.0f,
    bool disable_emission = false
) {
    shader.setVec4("spot_light.pos", spot_light.pos);
    shader.setVec4("spot_light.dir", spot_light.dir);
    shader.setVec4("spot_light.ambient", spot_light.ambient);
    shader.setVec4("spot_light.diffuse", spot_light.diffuse);
    shader.setVec4("spot_light.specular", spot_light.specular);
    shader.setVec3("spot_light.visibility", spot_light.visibility);
    shader.setFloat("spot_light.soft_cutoff", spot_light.soft_cutoff);
    shader.setFloat("spot_light.cutoff", spot_light.cutoff);

    for (int i = 0; i < 4; i++)
    {
        shader.setVec4(std::format("point_lights[{}].pos", i), point_lights[i].pos);
        shader.setVec4(std::format("point_lights[{}].ambient", i), point_lights[i].ambient);
        shader.setVec4(std::format("point_lights[{}].diffuse", i), point_lights[i].diffuse);
        shader.setVec4(std::format("point_lights[{}].specular", i), point_lights[i].specular);
        shader.setVec3(std::format("point_lights[{}].visibility", i), point_lights[i].visibility);
    }

    shader.setVec4("dir_light.dir", dir_light.dir);
    shader.setVec4("dir_light.ambient", dir_light.ambient);
    shader.setVec4("dir_light.diffuse", dir_light.diffuse);
    shader.setVec4("dir_light.specular", dir_light.specular);

    shader.setFloat("time", time);

    shader.setFloat("material.shininess", shininess);
}
#endif