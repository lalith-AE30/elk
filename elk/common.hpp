#ifndef COMMON_H
#define COMMON_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include "shader.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

struct Texture {
    unsigned int id = 0;
    std::string type;
    std::string path;
};

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

#define PHONG
#ifdef PHONG
// TODO Improve ABC and loc setting calls
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

    void update(Shader& shader) const;
};

class PointLight : public Light {
public:
    glm::vec4 pos;
    glm::vec3 visibility;
    unsigned int nr;

    PointLight();

    PointLight(
        glm::vec4 pos,
        glm::vec4 ambient,
        glm::vec4 diffuse,
        glm::vec4 specular,
        unsigned int nr = 0
    );

    void update(Shader& shader) const;
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
    );

    void update(Shader& shader) const;
};

#endif // PHONG

#endif // !COMMON_H

