#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>
#include <vector>
#include <memory>

#include "shader.hpp"

unsigned int loadTexture(const char*, bool = true, bool = false);

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

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    void draw(Shader& shader);

private:
    unsigned int VAO, VBO, EBO;

    void setupMesh();
};

class impl;

class Model {
public:
    Model(const char* path, bool vertically_flip_textures = true, bool use_alpha = false);

    ~Model();

    void draw(Shader& shader, int mesh_nr = -1);

private:
    std::unique_ptr<impl> pimpl;
};

#endif // !MODEL_LOADER_H
