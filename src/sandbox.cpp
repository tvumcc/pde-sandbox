#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "sandbox.hpp"
#include "heat.hpp"
#include "gray_scott.hpp"
#include "wave.hpp"
#include "navier_stokes.hpp"
#include "color_maps.hpp"

#include <iostream>

Sandbox::Sandbox(int window_width, int window_height, int gui_width) {
	grids.emplace_back(std::make_shared<Heat>(window_width, window_height));
	grids.emplace_back(std::make_shared<GrayScott>(window_width, window_height));
	grids.emplace_back(std::make_shared<Wave>(window_width, window_height));
	grids.emplace_back(std::make_shared<NavierStokes>(window_width, window_height));

    this->window_width = window_width;
    this->window_height = window_height;
    this->gui_width = gui_width;

    paused = false;
    pixelated = false;

    cmap = 1; // Set default color map to "Inferno"
    sim = 0; // Set default simulation to "Heat Equation"

    // Initialize the dropdown lists
    for (const auto& kp : cmaps) cmap_strs.push_back(kp.first.c_str());
    sim_strs.resize(3); 
    sim_strs = {"Heat Equation", "Gray-Scott Reaction Diffusion", "Wave Equation", "Navier-Stokes Fluid Flow"};
    boundary_condition_strs.resize(3); 
    boundary_condition_strs = {"Dirichlet", "Neumann", "Periodic"};

    reset_grid();
    reset_settings();
}

/**
 * Render the UI for the entire application
 */
void Sandbox::render_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x - gui_width, main_viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(gui_width, main_viewport->WorkSize.y));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    ImGui::Begin("Settings Menu", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // Simulation Section
    ImGui::SeparatorText("Simulation");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::Text("Simulation");
    if (ImGui::Combo("##Simulation", &sim, sim_strs.data(), sim_strs.size())) {
        reset_settings();
        reset_grid();
    }
    ImGui::Text("Resolution (Pixels per Cell)");
    if (ImGui::SliderInt("##Resolution (Pixels per Cell)", &grids[sim]->resolution, 1, 20))
        grids[sim]->resize((window_width - gui_width) / grids[sim]->resolution, window_height / grids[sim]->resolution);
    ImGui::Text("Space Step");    ImGui::SliderFloat("##Space Step", &grids[sim]->space_step, 0.1, 5.0);
    ImGui::Text("Time Step");     ImGui::SliderFloat("##Time Step", &grids[sim]->time_step, 0.01, 0.5);
    ImGui::Text("Boundary Condition"); ImGui::Combo("##Boundary Condition", &grids[sim]->boundary_condition, boundary_condition_strs.data(), boundary_condition_strs.size());

    // Control Buttons
    if (ImGui::Button(paused ? "Unpause" : "Pause")) paused = !paused;
    ImGui::SameLine(); if (ImGui::Button("Reset Grid")) grids[sim]->clear();
    ImGui::SameLine(); if (ImGui::Button("Reset Settings")) reset_settings();

    // Brush Section
    ImGui::SeparatorText("Brush");
    ImGui::Text("Brush Radius"); 
    ImGui::SliderInt("##Brush Radius", &(grids[sim]->brush_radius), 1, 100);

    // Visual Section
    ImGui::SeparatorText("Visual");
    ImGui::Text("Color Map");
    ImGui::Combo("##Color Map", &cmap, cmap_strs.data(), cmap_strs.size());
    if (ImGui::Checkbox("Pixelated", &pixelated))
        for (auto grid : grids) 
            grid->set_pixelated(pixelated);
    ImGui::PopItemWidth();


    // PDE specific section
    ImGui::SeparatorText(sim_strs[sim]);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    grids[sim]->gui();


    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
    ImGui::End();
}

/**
 *  Resizes all grids to the specified dimensions while also clearing them
 * 
 * @param window_width The new width of the application window
 * @param window_height The new height of the application window
 * @param gui_width The new width of the sidebar UI
 */
void Sandbox::resize(int window_width, int window_height, int gui_width) {
    this->window_width = window_width;
    this->window_height = window_height;
    this->gui_width = gui_width;

	for (int i = 0; i < grids.size(); i++)
		grids[i]->resize((window_width - gui_width) / grids[sim]->resolution, window_height / grids[sim]->resolution);
}

/**
 * Advances one time step in the simulation
 */
void Sandbox::advance_step() {
    grids[sim]->set_uniforms(cmap_strs[cmap], paused);
    grids[sim]->solve();
}

/**
 * Binds the currently selected grid
 */
void Sandbox::bind_current_grid() {
    grids[sim]->bind();
}

/**
 * Use the brush tool at the specified window coordinates
 * 
 * @param x_pos X coordinate in window space as taken from the mouse
 * @param y_pos Y coordinate in window space as taken from the mouse
 */
void Sandbox::brush(double x_pos, double y_pos) {
    grids[sim]->brush(
        (int)(x_pos / (window_width - gui_width) * grids[sim]->width), 
        (int)(y_pos / window_height * grids[sim]->height)
    );
}

/**
 * Resets settings for the currently selected grid to their defaults
 */
void Sandbox::reset_settings() {
    grids[sim]->reset_settings();
}

/**
 * Clears the currently selected grid
 */
void Sandbox::reset_grid() {
    grids[sim]->clear();
}