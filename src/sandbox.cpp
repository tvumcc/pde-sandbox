#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "sandbox.hpp"
#include "heat.hpp"
#include "gray_scott.hpp"
#include "wave.hpp"
#include "color_maps.hpp"

#include <iostream>

Sandbox::Sandbox(int width, int height) {
    // Initialize grids
	grids.emplace_back(std::make_shared<Heat>(width, height));
	grids.emplace_back(std::make_shared<GrayScott>(width, height));
	grids.emplace_back(std::make_shared<Wave>(width, height));

    window_width = width;
    window_height = height;
    gui_width = 320;

    paused = false;
    pixels = false;

    curr_cmap = 0;
    curr_sim = 0;
    curr_boundary_condition = 0;

    for (const auto& kp : cmaps) cmap_strs.push_back(kp.first.c_str());
    sim_strs.resize(3); sim_strs = {"Heat Equation", "Gray Scott Reaction Diffusion", "Wave Equation"};
    boundary_condition_strs.resize(3); boundary_condition_strs = {"Dirichlet", "Neumann", "Periodic"};

    reset_grid();
    reset_settings();
}

void Sandbox::render_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x - gui_width, main_viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(gui_width, main_viewport->WorkSize.y));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    ImGui::Begin("Settings Menu", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImGui::SeparatorText("Simulation");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        {
            ImGui::Text("Simulation");
            if (ImGui::Combo("##Simulation", &curr_sim, sim_strs.data(), sim_strs.size())) grid = grids[curr_sim];
        }
        {
            ImGui::Text("Resolution (Pixels per Cell)");
            if (ImGui::SliderInt("##Resolution (Pixels per Cell)", &resolution, 1, 20))
                for (int i = 0; i < grids.size(); i++)
                    grids[i]->resize((window_width - gui_width) / resolution, window_height / resolution);
            ImGui::Text("Space Step (dx)");
            ImGui::SliderFloat("##Space Step", &space_step, 0.1, 5.0);
            ImGui::Text("Time Step (dt)");
            ImGui::SliderFloat("##Time Step", &time_step, 0.01, 0.5);
        }
        {
            ImGui::Text("Brush Radius");
            ImGui::SliderInt("##Brush Radius", &brush_radius, 1, 100);
        }
        {
            ImGui::Text("Boundary Condition");
            ImGui::Combo("##Boundary Condition", &curr_boundary_condition, boundary_condition_strs.data(), boundary_condition_strs.size());
        }
        {
            if (ImGui::Button(paused ? "Unpause" : "Pause")) paused = !paused;
            ImGui::SameLine(); if (ImGui::Button("Reset Grid")) grids[curr_sim]->clear();
            ImGui::SameLine(); if (ImGui::Button("Reset Settings")) grids[curr_sim]->reset_settings();
        }
        ImGui::SeparatorText("Visual");
        {
            ImGui::Text("Color Map");
            ImGui::Combo("##Color Map", &curr_cmap, cmap_strs.data(), cmap_strs.size());
            if (ImGui::Checkbox("Pixelated", &pixels))
                for (auto grid : grids) 
                    grid->set_pixelated(pixels);
        }
        ImGui::PopItemWidth();
    }
    {
        ImGui::SeparatorText(sim_strs[curr_sim]);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        grids[curr_sim]->gui();
        ImGui::PopItemWidth();
    }
    ImGui::End();

    ImGui::PopStyleColor();

    // ImGui::ShowDemoWindow();
}

void Sandbox::resize(int width, int height) {
	for (int i = 0; i < grids.size(); i++)
		grids[i]->resize((width - gui_width) / resolution, height / resolution);
}

void Sandbox::advance_step() {
    grids[curr_sim]->set_uniforms(cmap_strs[curr_cmap], curr_boundary_condition, paused, space_step, time_step);
    grids[curr_sim]->solve();
}

void Sandbox::bind_current_grid() {
    grids[curr_sim]->bind();
}

void Sandbox::reset_settings() {
    brush_radius = 10;
    resolution = 8;
    space_step = 3.0;
    time_step = 0.1;

    grids[curr_sim]->reset_settings();
}

void Sandbox::reset_grid() {
    grids[curr_sim]->clear();
}