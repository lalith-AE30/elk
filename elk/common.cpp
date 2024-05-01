#include "common.hpp"

#ifdef PHONG
#include "shader_utils.hpp"

PointLight::PointLight() :
    pos(glm::vec4()), nr(0),
    visibility(getVisibility(50)),
    Light(glm::vec4(), glm::vec4(), glm::vec4()) {}

PointLight::PointLight(
    glm::vec4 pos,
    glm::vec4 ambient,
    glm::vec4 diffuse,
    glm::vec4 specular,
    unsigned int nr
) : pos(pos), nr(nr),
visibility(getVisibility(50)),
Light(ambient, diffuse, specular) {}

SpotLight::SpotLight(
    glm::vec4 pos,
    glm::vec4 dir,
    glm::vec4 ambient,
    glm::vec4 diffuse,
    glm::vec4 specular,
    float soft_cutoff,
    float cutoff
) :
    pos(pos),
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

void PointLight::update(Shader& shader) const {
    shader.setVec4(std::format("point_lights[{}].pos", nr), this->pos);
    shader.setVec4(std::format("point_lights[{}].ambient", nr), this->ambient);
    shader.setVec4(std::format("point_lights[{}].diffuse", nr), this->diffuse);
    shader.setVec4(std::format("point_lights[{}].specular", nr), this->specular);
    shader.setVec3(std::format("point_lights[{}].visibility", nr), this->visibility);
}

void DirectionalLight::update(Shader& shader) const {
    shader.setVec4("dir_light.dir", this->dir);
    shader.setVec4("dir_light.ambient", this->ambient);
    shader.setVec4("dir_light.diffuse", this->diffuse);
    shader.setVec4("dir_light.specular", this->specular);
}

void SpotLight::update(Shader& shader) const {
    shader.setVec4("spot_light.pos", this->pos);
    shader.setVec4("spot_light.dir", this->dir);
    shader.setVec4("spot_light.ambient", this->ambient);
    shader.setVec4("spot_light.diffuse", this->diffuse);
    shader.setVec4("spot_light.specular", this->specular);
    shader.setVec3("spot_light.visibility", this->visibility);
    shader.setFloat("spot_light.soft_cutoff", this->soft_cutoff);
    shader.setFloat("spot_light.cutoff", this->cutoff);
}
#endif // PHONG