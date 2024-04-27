#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

// TODO Remove dependent function so it can be made C compatible
#include "shader.hpp"

#include <format>
#include <vector>

struct Material {
    GLuint diffuse;
    GLuint specular;
    GLuint emission;
    float shininess;
};

inline glm::vec3 getVisibility(float distance) {
    return glm::vec3(1.0f, 4.5f / distance, 75.0f / (distance * distance));;
}

// TODO Improve ABC and loc setting calls
class Light {
public:
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    Light(
        glm::vec4 ambient,
        glm::vec4 diffuse,
        glm::vec4 specular
    ) : ambient(ambient),
        diffuse(diffuse),
        specular(specular) {}

    virtual void update(Shader& shader) const = 0;
    virtual ~Light() {}
};

class DirectionalLight : public Light {
public:
    glm::vec4 dir;

    DirectionalLight(
        glm::vec4 dir,
        glm::vec4 ambient,
        glm::vec4 diffuse,
        glm::vec4 specular
    ) : dir(dir),
        Light(ambient, diffuse, specular) {}

    void update(Shader& shader) const {
        shader.setVec4("dir_light.dir", this->dir);
        shader.setVec4("dir_light.ambient", this->ambient);
        shader.setVec4("dir_light.diffuse", this->diffuse);
        shader.setVec4("dir_light.specular", this->specular);
    }
};

class PointLight : public Light {
public:
    glm::vec4 pos;
    glm::vec3 visibility;
    unsigned int nr;

    PointLight() :
        pos(glm::vec4()), nr(0),
        visibility(getVisibility(50)),
        Light(glm::vec4(), glm::vec4(), glm::vec4()) {}

    PointLight(
        glm::vec4 pos,
        glm::vec4 ambient,
        glm::vec4 diffuse,
        glm::vec4 specular,
        unsigned int nr = 0
    ) : pos(pos), nr(nr),
        visibility(getVisibility(50)),
        Light(ambient, diffuse, specular) {}


    void update(Shader& shader) const {
        shader.setVec4(std::format("point_lights[{}].pos", nr), this->pos);
        shader.setVec4(std::format("point_lights[{}].ambient", nr), this->ambient);
        shader.setVec4(std::format("point_lights[{}].diffuse", nr), this->diffuse);
        shader.setVec4(std::format("point_lights[{}].specular", nr), this->specular);
        shader.setVec3(std::format("point_lights[{}].visibility", nr), this->visibility);
    }
};

class SpotLight : public Light {
public:
    glm::vec4 pos;
    glm::vec4 dir;
    float soft_cutoff;
    float cutoff;

    glm::vec3 visibility;

    SpotLight(
        glm::vec4 pos,
        glm::vec4 dir,
        glm::vec4 ambient,
        glm::vec4 diffuse,
        glm::vec4 specular,
        float soft_cutoff,
        float cutoff = -1
    ) : pos(pos),
        dir(dir),
        visibility(getVisibility(50)),
        soft_cutoff(soft_cutoff),
        Light(ambient, diffuse, specular)
    {
        if (cutoff == -1)
            this->cutoff = soft_cutoff + 0.01;
        else
            this->cutoff = cutoff;
    }

    void update(Shader& shader) const {
        shader.setVec4("spot_light.pos", this->pos);
        shader.setVec4("spot_light.dir", this->dir);
        shader.setVec4("spot_light.ambient", this->ambient);
        shader.setVec4("spot_light.diffuse", this->diffuse);
        shader.setVec4("spot_light.specular", this->specular);
        shader.setVec3("spot_light.visibility", this->visibility);
        shader.setFloat("spot_light.soft_cutoff", this->soft_cutoff);
        shader.setFloat("spot_light.cutoff", this->cutoff);
    }
};

inline void setVisibility(PointLight& light, float distance) {
    light.visibility = getVisibility(distance);
}
inline void setVisibility(SpotLight& light, float distance) {
    light.visibility = getVisibility(distance);
}

inline void updateMaterialShader(
    Shader& shader,
    std::vector<Light*> lights,
    float shininess = 32.0f,
    float time = 0.0f,
    bool disable_emission = false
) {
    for (auto& light : lights)
        light->update(shader);

    shader.setFloat("time", time);
    shader.setFloat("material.shininess", shininess);
}

#endif