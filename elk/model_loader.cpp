#include "model_loader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool missing_texture_loaded = false;
unsigned int missing_texture;

// TODO Add mesh generation from just vertex positions
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::draw(Shader& shader) {
    GLuint diffuse_nr = 0;
    GLuint specular_nr = 0;
    GLuint normal_nr = 0;
    shader.use();
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            diffuse_nr += 1;
            shader.setInt(std::format("material.texture_diffuse{}", diffuse_nr), i);
        }
        else if (name == "texture_specular") {
            specular_nr += 1;
            shader.setInt(std::format("material.texture_specular{}", specular_nr), i);
        }
        else if (name == "texture_normal") {
            normal_nr += 1;
            shader.setInt(std::format("material.texture_normal{}", normal_nr), i);
        }

        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    {
    if (specular_nr == 0) {
        if (!missing_texture_loaded) {
            missing_texture = loadTexture("models/missing_texture.png");
            missing_texture_loaded = true;
        }
        glActiveTexture(GL_TEXTURE10);
        shader.setInt("material.texture_specular1", 0);
        glBindTexture(GL_TEXTURE_2D, missing_texture);
    }
    if (normal_nr == 0) {
        if (!missing_texture_loaded) {
            missing_texture = loadTexture("models/missing_texture.png");
            missing_texture_loaded = true;
        }
        glActiveTexture(GL_TEXTURE10);
        shader.setInt("material.texture_normal1", 0);
        glBindTexture(GL_TEXTURE_2D, missing_texture);
    }
    if (textures.size() == 0) {
        if (!missing_texture_loaded) {
            missing_texture = loadTexture("models/missing_texture.png");
            missing_texture_loaded = true;
        }
        glActiveTexture(GL_TEXTURE0);

        shader.setInt("material.texture_diffuse1", 0);
        shader.setInt("material.texture_specular1", 0);

        glBindTexture(GL_TEXTURE_2D, missing_texture);
    }
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));

    glBindVertexArray(0);
}

class modelImpl {
private:
    std::vector<Texture> textures_loaded;
    std::string directory;
public:
    std::vector<Mesh> meshes;
    bool vertical_flip, use_alpha, use_normal_maps;

    modelImpl(const char* path, bool vertically_flip_textures, bool use_alpha, bool use_normal_maps) :
        vertical_flip(vertically_flip_textures),
        use_alpha(use_alpha),
        use_normal_maps(use_normal_maps) {
        loadModel(path);
    }

    void loadModel(std::string path) {
        Assimp::Importer import;
        const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        // Process per node for future local transforms
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex{};
            aiVector3f ai_pos = mesh->mVertices[i];
            vertex.pos.x = ai_pos.x;
            vertex.pos.y = ai_pos.y;
            vertex.pos.z = ai_pos.z;
            aiVector3f ai_norm = mesh->mNormals[i];
            vertex.normal.x = ai_norm.x;
            vertex.normal.y = ai_norm.y;
            vertex.normal.z = ai_norm.z;
            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec{};
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.tex_coord = vec;
            }
            else
                vertex.tex_coord = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            if (use_normal_maps) {
                std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
                textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            }
        }
        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string type_name) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = loadTexture(std::format("{}/{}", directory, str.C_Str()).c_str(), this->vertical_flip, true);
                texture.type = type_name;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

Model::Model(
    const char* path,
    bool vertically_flip_textures,
    bool use_alpha,
    bool use_normal_maps
) : pimpl(std::make_unique<modelImpl>(
    path,
    vertically_flip_textures,
    use_alpha,
    use_normal_maps
)) { }

void Model::draw(Shader& shader, int mesh_nr) {
    // TODO Add transforms to each mesh, as they currently all render at origin
    if (mesh_nr > -1 && mesh_nr < pimpl->meshes.size()) {
        pimpl->meshes[mesh_nr].draw(shader);
        return;
    }
    for (Mesh& mesh : pimpl->meshes) {
        mesh.draw(shader);
    }
}

unsigned int loadTexture(const char* filename, bool vertical_flip, bool use_alpha) {
    unsigned texture;
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
    if (vertical_flip)
        stbi_set_flip_vertically_on_load(true);
    else
        stbi_set_flip_vertically_on_load(false);

    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 3 + use_alpha);
    if (data) {
        if (use_alpha)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture " << filename << ". Reason: " << stbi_failure_reason() << std::endl;
        return 0;
    }
    stbi_image_free(data);
    return texture;
}

Model::~Model() { }