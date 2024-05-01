#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <format>
#include <vector>

#include "common.hpp"
#include "model_loader.hpp"
#include "shader.hpp"

struct Material {
    GLuint diffuse;
    GLuint specular;
    GLuint emission;
    float shininess;
};

inline glm::vec3 getVisibility(const float distance) {
    return glm::vec3(1.0f, 4.5f / distance, 75.0f / (distance * distance));;
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

inline void setVisibility(PointLight& light, const float distance) {
    light.visibility = getVisibility(distance);
}
inline void setVisibility(SpotLight& light, const float distance) {
    light.visibility = getVisibility(distance);
}

class RenderTarget {
private:
    GLuint fbo, rbo, tco;
    GLuint quad_vao, quad_vbo;

public:
    RenderTarget(const unsigned int scr_width, const unsigned int scr_height, const std::vector<Vertex>& target_mesh = std::vector<Vertex>()) {
        {
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

            glGenTextures(1, &tco);
            glBindTexture(GL_TEXTURE_2D, tco);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scr_width, scr_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tco, 0);

            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scr_width, scr_height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        {
            glGenVertexArrays(1, &quad_vao);
            glGenBuffers(1, &quad_vbo);
            glBindVertexArray(quad_vao);

            glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

            if (target_mesh.size() == 0) {
                float vertices[] = {
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                };

                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                // vertex positions
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                // vertex texture coords
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            }
            else {
                glBufferData(GL_ARRAY_BUFFER, target_mesh.size() * sizeof(Vertex), target_mesh.data(), GL_STATIC_DRAW);

                // vertex positions
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
                // vertex texture coords
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tex_coord)));
            }

            glBindVertexArray(0);
        }
    }

    ~RenderTarget() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteTextures(1, &tco);
    }

    void updateRenderShape(const unsigned int scr_width, const unsigned int scr_height) const {
        glBindTexture(GL_TEXTURE_2D, tco);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scr_width, scr_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tco, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scr_width, scr_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void use() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    void draw() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindVertexArray(quad_vao);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, tco);

        GLint prev_mode;
        glGetIntegerv(GL_POLYGON_MODE, &prev_mode);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glPolygonMode(GL_FRONT_AND_BACK, prev_mode);
    }
};
#endif