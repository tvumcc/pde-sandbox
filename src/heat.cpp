#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "heat.hpp"
#include "sandbox.hpp"

#include <iostream>

Heat::Heat(int width, int height) 
    : heatCS("shaders/heat.glsl"), Grid(width, height, 1)
{
    reset_settings();
}

/**
 * Dispatch the compute shader which solves the equation
 */
void Heat::solve() {
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for this specific equation
 */
void Heat::gui() {
    ImGui::Text("Diffusion");
    ImGui::SliderFloat("##Diffusion", &diffusion, 0.01, 3.0);
}

/**
 * Reset all simulation specific settings to default
 */
void Heat::reset_settings() {
    diffusion = 1.0;
    brush_radius = 10;
    space_step = 3.0f;
    time_step = 0.5f;
    boundary_condition = 1;
}

/**
 * Send the uniforms for this simulation to the compute shader
 * 
 * @param cmap_str String representing a color map to use
 * @param paused Is the simulation paused?
 */
void Heat::set_uniforms(std::string cmap_str, bool paused) {
    heatCS.bind();
    heatCS.set_bool("paused", paused);
    heatCS.set_int("width", width);
    heatCS.set_int("height", height);
    heatCS.set_int("boundary_condition", boundary_condition);
    heatCS.set_float("alpha", diffusion);
    heatCS.set_float("dx", space_step);
    heatCS.set_float("dt", time_step);

    heatCS.set_int("brush_enabled", brush_enabled);
    heatCS.set_int("x_pos", x_pos);
    heatCS.set_int("y_pos", y_pos);
    heatCS.set_int("brush_radius", brush_radius);
    apply_cmap(heatCS, cmap_str);
}