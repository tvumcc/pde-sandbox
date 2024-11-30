#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

#include "shader.hpp"
#include "grid.hpp"

#include <iostream>

GLFWwindow* window;
unsigned int WINDOW_WIDTH = 1000;
unsigned int WINDOW_HEIGHT = 800;
unsigned int VAO, VBO, EBO;
float r = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

void process_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup();

int main() {
	setup();

	ComputeShader cs("shaders/heat.glsl");
	cs.bind();

	Grid grid(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
	grid.brush(100, 100, 20, 0.5, 0);
	cs.set_int("width", grid.width);
	cs.set_int("height", grid.height);

	Shader shader("shaders/default.vert", "shaders/default.frag");
    shader.bind();
	shader.set_int("tex", 0);

	while (!glfwWindowShouldClose(window)) {
		process_input(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double x_pos, y_pos;
			glfwGetCursorPos(window, &x_pos, &y_pos);
			grid.brush((int)x_pos, (int)y_pos, 5, 1.0, 0);
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui Stuff Goes Here
		ImGui::Begin("Menu");
		ImGui::Text("Hi");
		ImGui::End();

		cs.bind();
		glDispatchCompute(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grid.image);

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
	glViewport(0, 0, width, height);
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, texture);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
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