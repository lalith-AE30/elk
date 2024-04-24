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

	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

	GLFWwindow* window = bindWindow(800, 600, &camera, "Model Viwer");

	// TODO Remove bindings control from main loop to window handler.
	Bindings bindings = generate_bindings(window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	Shader lights_shader("shaders/phong.vert", "shaders/phong.frag");
	Shader depth_shader("shaders/phong.vert", "shaders/depth.frag");
	Shader normal_shader("shaders/normal.vert", "shaders/normal.frag");
	Shader outline_shader("shaders/phong.vert", "shaders/outline.frag");
	Shader light_source_shader("shaders/light.vert", "shaders/light.frag");

	Shader* active_shader = &lights_shader;
	active_shader = &depth_shader;

	Model bulb("models/sphere/sphere.obj", true);
	Model grass("models/grass/grass.obj", false, true);
	Model model_3d1("models/skull/skull.obj", false);
	Model model_3d("models/chess_board/chess_board.obj", true);

	int nr_grass = 10;
	std::vector<glm::vec2> dist;
	dist.resize(nr_grass);
	for (int i = 0; i < nr_grass; i++) {
		dist[i] = glm::diskRand(5.0f);
	}

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

	int nr_lights = 4;
	std::vector<PointLight> point_lights;
	point_lights.resize(nr_lights);
	for (int i = 0; i < nr_lights; i++) {
		point_lights[i] = point_light;
		point_lights[i].pos = glm::vec4(point_light_positions[i], 1.0f);
	}

	float dt = 0.0f;
	float last_frame = 0.0f;

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glm::vec4 clear_color(0.0f);
	float scale = 1.0f;
	int active_shader_type = 0;
	bool render_outline = false;

	while (!glfwWindowShouldClose(window)) {
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
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			static int counter = 0;

			ImGui::Begin("Dashboard");

			ImGui::Text("Text Test");

			ImGui::ColorEdit3("clear color", (float*)&clear_color);

			ImGui::SliderFloat("Scale", &scale, 0.1, 10.0);

			ImGui::Checkbox("Render outline", &render_outline);

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

			if (ImGui::Button("Reload Skull"))
				Model model_3d1("models/skull/skull.obj");
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		ImGui::Render();
		glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		float aspect = (float)state.scr_width / (float)state.scr_height;
		glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), aspect, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		active_shader->use();

		active_shader->setMat4("proj", proj);
		active_shader->setMat4("view", view);

		dir_light.dir = glm::vec4(sin(current_frame), -1.0f, cos(current_frame), 0.0f);
		updateMaterialShader(*active_shader, spot_light, point_lights, dir_light);

		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));

			glStencilMask(0x00);
			active_shader->setMat4("model", model);

			model_3d.draw(*active_shader, state.mesh);
		}

		for (int i = 0; i < nr_grass; i++) {
			glm::mat4 model(1.0f);
			model = glm::translate(model, glm::vec3(dist[i].x, 0.7f, dist[i].y));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(scale));
			active_shader->setMat4("model", model);
			grass.draw(*active_shader, state.mesh);
		}

		{
			glStencilMask(0xFF);

			glm::mat4 model(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));

			active_shader->setMat4("model", model);

			model_3d1.draw(*active_shader, state.mesh);

			if (render_outline) {
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glDisable(GL_DEPTH_TEST);
				glStencilMask(0x00);

				model = glm::scale(model, glm::vec3(1.05f, 1.05f, 1.05f));

				outline_shader.use();

				outline_shader.setMat4("proj", proj);
				outline_shader.setMat4("view", view);
				outline_shader.setMat4("model", model);

				model_3d1.draw(outline_shader, state.mesh);

				glEnable(GL_DEPTH_TEST);
				glStencilMask(0xFF);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
			}
		}

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

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}