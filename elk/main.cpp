#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "camera.hpp"
#include "model_loader.hpp"
#include "shader.hpp"
#include "shader_utils.hpp"
#include "window_callbacks.hpp"

int main() {
    glfwInit();
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    // Initialize windows and attach camera -----------------------------------------------------
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    GLFWwindow* window = bindWindow(800, 600, &camera, "Model Viwer");
    // TODO Remove bindings control from main loop to window handler.
    Bindings bindings = generate_bindings(window);
    // ------------------------------------------------------------------------------------------

    // Initialize ImGUI -------------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    {
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }
    // ------------------------------------------------------------------------------------------

    // Initialize shaders -----------------------------------------------------------------------
    Shader lights_shader("shaders/phong.vert", "shaders/phong.frag");
    //Shader lights_shader("shaders/phong_normal.vert", "shaders/phong_normal.frag");
    Shader depth_shader("shaders/phong.vert", "shaders/depth.frag");
    Shader normal_shader("shaders/normal.vert", "shaders/normal.frag");
    Shader outline_shader("shaders/phong.vert", "shaders/outline.frag");
    Shader light_source_shader("shaders/light.vert", "shaders/light.frag");
    Shader screen_shader("shaders/screen.vert", "shaders/screen_sharpen.frag");
    // ------------------------------------------------------------------------------------------

    // Initialize Models ------------------------------------------------------------------------
    Model test_object("models/anime_girl/anime_girl.obj", false, false);
    Model bulb("models/sphere/sphere.obj", true);
    Model grass("models/grass/grass.obj", false, true);
    Model chess_board("models/chess_board/chess_board.obj", true, false);
    // ------------------------------------------------------------------------------------------

    // Initialize Lights ------------------------------------------------------------------------
    DirectionalLight dir_light = {
        .dir = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f),
        .ambient = glm::vec4(0.02f, 0.01f, 0.005f, 1.0f),
        .diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
        .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };
    PointLight point_light = {
        .pos = glm::vec4(1.2f, 1.0f, 2.0f, 1.0f),
        .ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        .diffuse = glm::vec4(0.7f, 0.2f, 0.2f, 1.0f),
        .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };
    SpotLight spot_light = {
     .pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
     .dir = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f),
     .soft_cutoff = glm::cos(glm::radians(10.0f)),
     .cutoff = glm::cos(glm::radians(11.0f)),
     .ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
     .diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
     .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };

    glm::vec3 point_light_positions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };
    std::vector<PointLight> point_lights;
    // ------------------------------------------------------------------------------------------

    // Attach pointers and transforms -----------------------------------------------------------
    std::vector<glm::vec2> dist;

    Shader* active_shader = &lights_shader;
    active_shader = &depth_shader;

    int nr_grass = 10;
    {
        dist.resize(nr_grass);
        for (int i = 0; i < nr_grass; i++) {
            dist[i] = glm::diskRand(5.0f);
        }
    }

    int nr_lights = 4;
    {
        point_lights.resize(nr_lights);
        for (int i = 0; i < nr_lights; i++) {
            point_lights[i] = point_light;
            point_lights[i].pos = glm::vec4(point_light_positions[i], 1.0f);
        }
    }
    // ------------------------------------------------------------------------------------------

    // Build framebuffer & Render Target --------------------------------------------------------
    GLuint fbo, rbo, tco;
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &tco);
        glBindTexture(GL_TEXTURE_2D, tco);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tco, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint quad_vao, quad_vbo;
    {
        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &quad_vbo);
        glBindVertexArray(quad_vao);

        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

        float vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        // vertex texture coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
    }
    // ------------------------------------------------------------------------------------------

    // Enable buffer-based effects and optimizations --------------------------------------------
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_CULL_FACE);
    // ------------------------------------------------------------------------------------------

    // Render loop state ------------------------------------------------------------------------
    glm::vec4 clear_color(0.0f);
    float scale = 1.0f, dt = 0.0f, last_frame = 0.0f;
    int active_shader_type = 0, culling = 2, polygon_mode = 0, prev_poly_mode = polygon_mode;
    bool vsync = true,
        render_outline = false,
        render_grass = true;
    // ------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        // Update scene state -------------------------------------------------------------------
        float current_frame = static_cast<float>(glfwGetTime());
        dt = current_frame - last_frame;
        last_frame = current_frame;

        WindowState state = controller::getState();
        {
            if (!io.WantCaptureMouse) {
                controller::processInput(window, &bindings, dt);
                for (auto& point_light : point_lights)
                    setVisibility(point_light, state.distance);
                setVisibility(spot_light, state.distance);
            }
            if (state.framesize_changed) {
                glBindTexture(GL_TEXTURE_2D, tco);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.scr_width, state.scr_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glBindTexture(GL_TEXTURE_2D, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tco, 0);

                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.scr_width, state.scr_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }
        // ------------------------------------------------------------------------------------------

        // GUI --------------------------------------------------------------------------------------
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Dashboard");
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Text("Text Test");

            glm::vec4 last_color = clear_color;
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (last_color != clear_color)
                glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);

            ImGui::SliderFloat("Scale", &scale, 0.1f, 10.0);

            ImGui::Checkbox("Render outline", &render_outline); ImGui::SameLine();
            ImGui::Checkbox("Render grass", &render_grass);

            ImGui::RadioButton("Phong", &active_shader_type, 0); ImGui::SameLine();
            ImGui::RadioButton("Depth", &active_shader_type, 1); ImGui::SameLine();
            ImGui::RadioButton("Normal", &active_shader_type, 2);

            switch (active_shader_type)
            {
            case 0:
                active_shader = &lights_shader;
                break;
            case 1:
                active_shader = &depth_shader;
                break;
            case 2:
                active_shader = &normal_shader;
                break;
            default:
                break;
            }

            int prev_cull = culling;

            ImGui::RadioButton("No Culling", &culling, 0); ImGui::SameLine();
            ImGui::RadioButton("Front face", &culling, 1); ImGui::SameLine();
            ImGui::RadioButton("Back face", &culling, 2);

            switch (culling) {
            case 0:
                if (prev_cull)
                    glDisable(GL_CULL_FACE);
                break;
            case 1:
                if (prev_cull == 0)
                    glEnable(GL_CULL_FACE);
                if (prev_cull != 1)
                    glCullFace(GL_FRONT);
                break;
            case 2:
                if (prev_cull == 0)
                    glEnable(GL_CULL_FACE);
                if (prev_cull != 2)
                    glCullFace(GL_BACK);
                break;
            default:
                break;
            }

            prev_poly_mode = polygon_mode;

            ImGui::RadioButton("Fill", &polygon_mode, 0); ImGui::SameLine();
            ImGui::RadioButton("Lines", &polygon_mode, 1); ImGui::SameLine();
            ImGui::RadioButton("Points", &polygon_mode, 2);

            switch (polygon_mode) {
            case 0:
                if (prev_poly_mode)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            case 1:
                if (prev_poly_mode != 1)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case 2:
                if (prev_poly_mode != 2)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;
            default:
                break;
            }

            ImGui::Checkbox("Enable VSync", &vsync);
            glfwSwapInterval(vsync);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        ImGui::Render();
        // ------------------------------------------------------------------------------------------

        // Set-up next frame ------------------------------------------------------------------------
        active_shader->use();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        float aspect = (float)state.scr_width / (float)state.scr_height;
        glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), aspect, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();

        active_shader->setMat4("proj", proj);
        active_shader->setMat4("view", view);

        dir_light.dir = glm::vec4(sin(current_frame), -1.0f, cos(current_frame), 0.0f);
        updateMaterialShader(*active_shader, spot_light, point_lights, dir_light);
        // ------------------------------------------------------------------------------------------

        // Render Chessboard ------------------------------------------------------------------------
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));

            glStencilMask(0x00);
            active_shader->setMat4("model", model);

            chess_board.draw(*active_shader, state.mesh);
        }
        // ------------------------------------------------------------------------------------------

        // Billboard Grass --------------------------------------------------------------------------
        for (int i = 0; render_grass && i < nr_grass; i++) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(dist[i].x, 0.7f, dist[i].y));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            active_shader->setMat4("model", model);
            grass.draw(*active_shader, state.mesh);
        }
        // ------------------------------------------------------------------------------------------

        // Test Object ------------------------------------------------------------------------------
        {
            glStencilMask(0xFF);

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));
            model = glm::scale(model, glm::vec3(scale));

            active_shader->setMat4("model", model);

            test_object.draw(*active_shader, state.mesh);

            if (render_outline) {
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glDisable(GL_DEPTH_TEST);
                glStencilMask(0x00);

                model = glm::scale(model, glm::vec3(1.05f, 1.05f, 1.05f));

                outline_shader.use();

                outline_shader.setMat4("proj", proj);
                outline_shader.setMat4("view", view);
                outline_shader.setMat4("model", model);

                test_object.draw(outline_shader, state.mesh);

                glEnable(GL_DEPTH_TEST);
                glStencilMask(0xFF);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
            }
        }
        // ------------------------------------------------------------------------------------------

        // Lights -----------------------------------------------------------------------------------
        {
            light_source_shader.use();

            light_source_shader.setMat4("proj", proj);
            light_source_shader.setMat4("view", view);

            light_source_shader.setVec4("light_color", point_light.diffuse * 1e-1f * state.distance); // TODO Change heuristic constant

            for (auto& pos : point_light_positions) {
                glm::mat4 model(1.0f);
                model = glm::translate(model, pos);
                light_source_shader.setMat4("model", model);
                bulb.draw(light_source_shader);
            }
        }
        // ------------------------------------------------------------------------------------------

        // Render to target -------------------------------------------------------------------------
        screen_shader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(quad_vao);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, tco);

        if (prev_poly_mode != 0)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        if (prev_poly_mode != 0 && polygon_mode!=0)
            glPolygonMode(GL_FRONT_AND_BACK, prev_poly_mode == 1 ? GL_LINE : GL_POINT);
        // ------------------------------------------------------------------------------------------

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &tco);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}