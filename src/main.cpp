#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

#include "shader.hpp"
#include "grid.hpp"
#include "heat.hpp"
#include "gray_scott.hpp"
#include "wave.hpp"
#include "color_maps.hpp"

#include <iostream>
#include <vector>
#include <memory>

GLFWwindow* window;
unsigned int WINDOW_WIDTH = 1000;
unsigned int WINDOW_HEIGHT = 800;
unsigned int GUI_WIDTH = 300;
unsigned int VAO, VBO, EBO;
float r = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

const char* cmap_strs[] = {"Viridis", "Blues_r"};
int current_cmap = 0;
const char* sims[] = {"Heat Equation", "Gray Scott Reaction Diffusion", "Wave Equation"};
int current_sim = 0;
const char* boundary_conditions[] = {"Dirichlet", "Neumann", "Periodic"};
int current_boundary_condition = 0;

std::vector<std::unique_ptr<Grid>> pdes;

int brush_radius = 10;
int pixels_per_cell = 8;
float space_step = 3.0;
float time_step = 0.1;
bool paused = false;
bool pixels = false;

void process_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup();

int main() {
	setup();

	pdes.emplace_back(std::make_unique<Heat>(WINDOW_WIDTH, WINDOW_HEIGHT));
	pdes.emplace_back(std::make_unique<GrayScott>(WINDOW_WIDTH, WINDOW_HEIGHT));
	pdes.emplace_back(std::make_unique<Wave>(WINDOW_WIDTH, WINDOW_HEIGHT));
	framebuffer_size_callback(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	Shader shader("shaders/default.vert", "shaders/default.frag");
    shader.bind();
	shader.set_int("tex", 0);

	while (!glfwWindowShouldClose(window)) {
		process_input(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		pdes[current_sim]->bind();

		// ImGui Stuff Goes Here
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x - GUI_WIDTH, main_viewport->WorkPos.y));
		ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, main_viewport->WorkSize.y));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

		ImGui::Begin("Settings Menu", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		{
			ImGui::SeparatorText("General Settings");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			{
				ImGui::Text("Simulation");
				ImGui::Combo("##Simulation", &current_sim, sims, IM_ARRAYSIZE(sims));
			}
			{
				ImGui::Text("Color Map");
				ImGui::Combo("##Color Map", &current_cmap, cmap_strs, IM_ARRAYSIZE(cmap_strs));
				if (ImGui::Checkbox("Pixelated", &pixels)) {
					for (int i = 0; i < pdes.size(); i++)
						pdes[i]->set_pixelated(pixels);
				}
			}
			{
				ImGui::Text("Pixels per Cell");
				if (ImGui::SliderInt("##Pixels per Cell", &pixels_per_cell, 1, 20)) {
					for (int i = 0; i < pdes.size(); i++)
						pdes[i]->resize((WINDOW_WIDTH - GUI_WIDTH) / pixels_per_cell, WINDOW_HEIGHT / pixels_per_cell);
				}
				ImGui::Text("Space Step");
				ImGui::SliderFloat("##Space Step", &space_step, 0.1, 5.0);
				ImGui::Text("Time Step");
				ImGui::SliderFloat("##Time Step", &time_step, 0.01, 0.5);
			}
			{
				ImGui::Text("Brush Radius");
				ImGui::SliderInt("##Brush Radius", &brush_radius, 1, 100);
			}
			{
				ImGui::Text("Boundary Condition");
				ImGui::Combo("##Boundary Condition", &current_boundary_condition, boundary_conditions, IM_ARRAYSIZE(boundary_conditions));
			}
			{
				if (ImGui::Button(paused ? "Unpause" : "Pause")) paused = !paused;
				ImGui::SameLine();
				if (ImGui::Button("Reset")) pdes[current_sim]->clear();
			}
			ImGui::PopItemWidth();
		}
		{
			ImGui::SeparatorText(sims[current_sim]);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			pdes[current_sim]->gui();
			ImGui::PopItemWidth();
		}
		ImGui::End();

		ImGui::PopStyleColor();

		// ImGui::ShowDemoWindow();

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double x_pos, y_pos;
			glfwGetCursorPos(window, &x_pos, &y_pos);
			pdes[current_sim]->brush((int)(x_pos / (WINDOW_WIDTH - GUI_WIDTH) * pdes[current_sim]->width), (int)(y_pos / WINDOW_HEIGHT * pdes[current_sim]->height), brush_radius, 1.0);
		}

		pdes[current_sim]->set_uniforms(cmap_strs[current_cmap], current_boundary_condition, paused, space_step, time_step);
		pdes[current_sim]->solve();

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pdes[current_sim]->image);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		ImGui::Render();
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

void process_input(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);	
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;

	glViewport(0, 0, WINDOW_WIDTH - GUI_WIDTH, WINDOW_HEIGHT);
	for (int i = 0; i < pdes.size(); i++)
		pdes[i]->resize((WINDOW_WIDTH - GUI_WIDTH) / pixels_per_cell, WINDOW_HEIGHT / pixels_per_cell);
	// for (int i = 0; i < pdes.size(); i++)
	// 	pdes[i]->resize(500, 400);
}

void setup() {
	float vertices[] = {
		 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,   // top right
		 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,   // bottom right
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,   // bottom left
		-1.0f,  1.0f, 0.0f, 0.0f, 0.0f    // top left 
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Partial Differential Equations Simulation", NULL, NULL);
	glfwMakeContextCurrent(window);
	GLFWimage icons[2];
	icons[0].pixels = stbi_load("assets/laplacian.png", &icons[0].width, &icons[0].height, 0, 4);
	icons[1].pixels = stbi_load("assets/laplacian_small.png", &icons[1].width, &icons[1].height, 0, 4);
	if (icons[0].pixels != NULL && icons[1].pixels != NULL) glfwSetWindowIcon(window, 2, icons);
	stbi_image_free(icons[0].pixels);
	stbi_image_free(icons[1].pixels);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	std::cout << "Current Version: " << glGetString(GL_VERSION) << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}