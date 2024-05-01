#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>
#include <vector>
#include <memory>

#include "shader.hpp"
#include "common.hpp"

GLuint loadTexture(const char*, bool = true, bool = false);
GLuint loadCubemap(std::vector<std::string>& faces);

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<float> vertex_positions);
    
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    void draw(Shader& shader);

private:
    unsigned int vao, vbo, ebo;

    void setupMesh();
};

class modelImpl;

class Model {
public:
    Model(const char* path, bool vertically_flip_textures = true, bool use_alpha = false, bool use_normal_maps = false);

    ~Model();

    void draw(Shader& shader, int mesh_nr = -1);

    std::vector<Mesh>& getMeshes();

private:
    std::unique_ptr<modelImpl> pimpl;
};

class Skybox {
private:
    GLuint skybox_vao, skybox_vbo;
    Shader skybox_shader;

public:
    GLuint cubemap_texture;

    Skybox(const char* vert_path, const char* frag_path, std::vector<std::string>& face_paths);

    void draw(glm::mat4& proj, glm::mat4& view);
};

#endif // !MODEL_LOADER_H
