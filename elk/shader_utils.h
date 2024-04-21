#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

void updateMaterialShader(Shader& shader, Material& material, DirectionalLight& light, float time, bool disable_emission = false) {
    shader.setVec4("light.dir", light.dir);
    shader.setVec4("light.ambient", light.ambient);
    shader.setVec4("light.diffuse", light.diffuse);
    shader.setVec4("light.specular", light.specular);

    shader.setFloat("time", time);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular);
    if (!disable_emission) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.emission);
    }
    shader.setFloat("material.shininess", material.shininess);
}
void updateMaterialShader(Shader& shader, Material& material, PointLight& light, float time, bool disable_emission = false) {
    shader.setVec4("light.pos", light.pos);
    shader.setVec4("light.ambient", light.ambient);
    shader.setVec4("light.diffuse", light.diffuse);
    shader.setVec4("light.specular", light.specular);

    shader.setVec3("light.visibility", light.visibility);

    shader.setFloat("time", time);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular);
    if (!disable_emission) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.emission);
    }
    shader.setFloat("material.shininess", material.shininess);
}
void updateMaterialShader(Shader& shader, Material& material, SpotLight& light, float time, bool disable_emission = false) {
    shader.setVec4("spot_light.pos", light.pos);
    shader.setVec4("spot_light.dir", light.dir);
    shader.setFloat("spot_light.soft_cutoff", light.soft_cutoff);
    shader.setFloat("spot_light.cutoff", light.cutoff);
    shader.setVec4("spot_light.ambient", light.ambient);
    shader.setVec4("spot_light.diffuse", light.diffuse);
    shader.setVec4("spot_light.specular", light.specular);

    shader.setVec3("spot_light.visibility", light.visibility);

    shader.setFloat("time", time);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular);
    if (!disable_emission) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.emission);
    }
    shader.setFloat("material.shininess", material.shininess);
}
void updateMaterialShader(
    Shader& shader,
    Material& material,
    SpotLight& spot_light,
    PointLight point_lights[],
    DirectionalLight& dir_light,
    float time,
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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular);
    if (!disable_emission) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.emission);
    }
    shader.setFloat("material.shininess", material.shininess);
}

GLuint LoadTexture(const char* filename) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 3);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }
    stbi_image_free(data);
    return texture;
}

#endif