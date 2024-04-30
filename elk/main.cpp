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

class Scene {
	Scene(std::vector<Model> models);
};

class Skybox {
private:
	GLuint skybox_vao, skybox_vbo;
	GLuint cubemap_texture;
	Shader skybox_shader;

public:
	Skybox(const char* vert_path, const char* frag_path, std::vector<std::string>& face_paths) : skybox_shader(vert_path, frag_path) {
		cubemap_texture = loadCubemap(face_paths);
		float skybox_vertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
		glGenVertexArrays(1, &skybox_vao);
		glGenBuffers(1, &skybox_vbo);

		glBindVertexArray(skybox_vao);
		glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(0);
	}

	void draw(glm::mat4& proj, glm::mat4& view) {
		glDepthMask(GL_FALSE);
		skybox_shader.use();

		skybox_shader.setMat4("proj", proj);
		skybox_shader.setMat4("view", glm::mat4(glm::mat3(view)));

		glBindVertexArray(skybox_vao);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
	}
};

std::vector<Vertex> generateSquareVertices(float x) {
	std::vector<Vertex> vertices = {
		{{x, x, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{1.0f, x, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{x, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{x, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{1.0f, x, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	};
	return vertices;
}

int main() {
	glfwInit();
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	// Initialize windows and attach camera -----------------------------------------------------
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	GLFWwindow* window = bindWindow(1280, 720, &camera, "Model Viwer");
	Bindings bindings = generate_bindings(window);

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

	// Initialize shaders -----------------------------------------------------------------------
	Shader lights_shader("shaders/phong.vert", "shaders/phong.frag");
	Shader depth_shader("shaders/phong.vert", "shaders/depth.frag");
	Shader normal_shader("shaders/normal.vert", "shaders/normal.frag");
	Shader outline_shader("shaders/phong.vert", "shaders/outline.frag");
	Shader light_source_shader("shaders/light.vert", "shaders/light.frag");
	Shader screen_shader("shaders/screen.vert", "shaders/screen_postprocess.frag");
	Shader mandelbrot_shader("shaders/screen.vert", "shaders/mandelbrot.frag");

	// Initialize Models ------------------------------------------------------------------------
	Model test_object("models/backpack/backpack.obj", true, false);
	Model bulb("models/sphere/sphere.obj", true);
	Model grass("models/grass/grass.obj", false, true);
	Model chess_board("models/chess_board/chess_board.obj", true, false);

	std::vector<std::string> faces = {
		"skybox/milkyway/px.png",
		"skybox/milkyway/nx.png",
		"skybox/milkyway/py.png",
		"skybox/milkyway/ny.png",
		"skybox/milkyway/pz.png",
		"skybox/milkyway/nz.png"
	};
	Skybox skybox("shaders/skybox.vert", "shaders/skybox.frag", faces);

	// Initialize Lights ------------------------------------------------------------------------
	DirectionalLight dir_light(
		glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f),
		glm::vec4(0.02f, 0.01f, 0.005f, 1.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	);
	SpotLight spot_light(
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 0.0f, -1.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::cos(glm::radians(10.0f)),
		glm::cos(glm::radians(11.0f))
	);

	glm::vec3 point_light_positions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
	std::vector<PointLight*> point_lights;

	// Attach pointers and transforms -----------------------------------------------------------
	std::vector<glm::vec2> dist;

	Shader* active_shader = &lights_shader;
	active_shader = &depth_shader;

	int nr_grass = 10, nr_lights = 4;
	{
		dist.resize(nr_grass);
		for (int i = 0; i < nr_grass; i++) {
			dist[i] = glm::diskRand(5.0f);
		}

		point_lights.resize(nr_lights);
		for (int i = 0; i < nr_lights; i++) {
			PointLight* point_light = new PointLight(
				glm::vec4(1.2f, 1.0f, 2.0f, 1.0f),
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
				glm::vec4(0.7f, 0.2f, 0.2f, 1.0f),
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
			);
			point_lights[i] = point_light;
			point_lights[i]->pos = glm::vec4(point_light_positions[i], 1.0f);
			point_lights[i]->nr = i;
		}
	}

	std::vector<Light*> lights = {
		&dir_light,
		&spot_light
	};
	lights.insert(lights.end(), point_lights.begin(), point_lights.end());

	// Build framebuffer & Render Target --------------------------------------------------------
	RenderTarget target(1280, 720);
	RenderTarget mandel(256, 144, generateSquareVertices(0.6));

	// Enable buffer-based effects and optimizations --------------------------------------------
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_CULL_FACE);

	// Render loop state ------------------------------------------------------------------------
	glm::vec4 clear_color(0.0f);
	float scale = 1.0f, dt = 0.0f, last_frame = 0.0f;
	int active_shader_type = 0, culling = 2, polygon_mode = 0, prev_poly_mode = polygon_mode;
	bool vsync = true,
		render_outline = false,
		render_grass = true;

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
					setVisibility(*point_light, state.distance);
				setVisibility(spot_light, state.distance);
			}
			if (state.framesize_changed)
				target.updateRenderShape(state.scr_width, state.scr_height);
		}

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
			ImGui::Render();
		}

		// Set-up next frame and render all objects for target1 ------------------------------------
		{
			float aspect = (float)state.scr_width / (float)state.scr_height;
			glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), aspect, 0.1f, 100.0f);
			glm::mat4 view = camera.getViewMatrix();

			{
				target.use();
				glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
				glEnable(GL_DEPTH_TEST);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				active_shader->use();
				active_shader->setMat4("proj", proj);
				active_shader->setMat4("view", view);

				dir_light.dir = glm::vec4(sin(current_frame), -1.0f, cos(current_frame), 0.0f);
				updateMaterialShader(*active_shader, lights);
			}

			{
				// Render Chessboard --------------------------------------------------------------------
				{
					glm::mat4 model(1.0f);
					model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));

					glStencilMask(0x00);
					active_shader->setMat4("model", model);

					chess_board.draw(*active_shader, state.mesh);
				}
				// --------------------------------------------------------------------------------------

				// Billboard Grass ----------------------------------------------------------------------
				for (int i = 0; render_grass && i < nr_grass; i++) {
					glm::mat4 model(1.0f);
					model = glm::translate(model, glm::vec3(dist[i].x, 0.7f, dist[i].y));
					model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
					active_shader->setMat4("model", model);
					grass.draw(*active_shader, state.mesh);
				}
				// --------------------------------------------------------------------------------------

				// Test Object --------------------------------------------------------------------------
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
				// --------------------------------------------------------------------------------------

				// Lights -------------------------------------------------------------------------------
				{
					light_source_shader.use();

					light_source_shader.setMat4("proj", proj);
					light_source_shader.setMat4("view", view);

					light_source_shader.setVec4("light_color", point_lights[0]->diffuse * 1e-1f * state.distance); // TODO Change heuristic constant

					for (auto& pos : point_light_positions) {
						glm::mat4 model(1.0f);
						model = glm::translate(model, pos);
						light_source_shader.setMat4("model", model);
						bulb.draw(light_source_shader);
					}
				}
			}

			skybox.draw(proj, view);
		}

		// Render to target -------------------------------------------------------------------------
		screen_shader.use();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		target.draw();
		mandelbrot_shader.use();
		mandelbrot_shader.setFloat("time", current_frame);
		mandel.draw();

		// New frame setup --------------------------------------------------------------------------
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	for (auto& point_light : point_lights)
		delete point_light;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}