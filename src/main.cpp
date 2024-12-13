#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

#include "shader.hpp"
#include "sandbox.hpp"

#include <iostream>
#include <vector>
#include <memory>

GLFWwindow* window;
unsigned int WINDOW_WIDTH = 1000;
unsigned int WINDOW_HEIGHT = 800;
unsigned int GUI_WIDTH = 320;
unsigned int VAO, VBO, EBO;

void setup();

int main() {
	setup();
	Sandbox sandbox = Sandbox(WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetWindowUserPointer(window, &sandbox);
	auto resize_window = [](GLFWwindow* window, int width, int height){
		WINDOW_WIDTH = width;
		WINDOW_HEIGHT = height;
		Sandbox* sandbox = (Sandbox*)glfwGetWindowUserPointer(window);

		glViewport(0, 0, WINDOW_WIDTH - sandbox->gui_width, WINDOW_HEIGHT);
		sandbox->resize(width, height);
	};
    glfwSetFramebufferSizeCallback(window, resize_window);
	resize_window(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	Shader shader("shaders/default.vert", "shaders/default.frag");
    shader.bind();
	shader.set_int("tex", 0);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		sandbox.bind_current_grid();
		sandbox.render_gui();

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double x_pos, y_pos;
			glfwGetCursorPos(window, &x_pos, &y_pos);
			sandbox.grids[sandbox.curr_sim]->brushGaussian((int)(x_pos / (WINDOW_WIDTH - GUI_WIDTH) * sandbox.grids[sandbox.curr_sim]->width), (int)(y_pos / WINDOW_HEIGHT * sandbox.grids[sandbox.curr_sim]->height), sandbox.brush_radius, 1.0);
		}

		sandbox.advance_step();


		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sandbox.grids[sandbox.curr_sim]->image);

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
	std::cout << "Current Version: " << glGetString(GL_VERSION) << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	io.Fonts->AddFontFromFileTTF("assets/NotoSans.ttf", 25);
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