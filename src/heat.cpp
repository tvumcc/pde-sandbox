#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "heat.hpp"
#include "sandbox.hpp"

#include <iostream>

Heat::Heat(int width, int height) 
    : heatCS("shaders/heat.glsl"), Grid(width, height, 1, 0.0f)
{
    reset_settings();
}

/**
 * Dispatch the compute shader which solves the equation
 */
void Heat::solve() {
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
    glDispatchCompute(this->width, this->height, 1);
}

/**
 * Render the GUI for the Heat Equation Simulation using ImGui
 */
void Heat::gui() {
    ImGui::Text("Diffusion");
    ImGui::SliderFloat("##Diffusion", &diffusion, 0.01, 3.0);
}

/**
 * Reset all simulation specific settings to default
 */
void Heat::reset_settings() {
    this->diffusion = 1.0;
    this->brush_radius = 10;
    this->brush_type = 0;
}

/**
 * 
 */
void Heat::use_recommended_settings(Sandbox& sandbox) {
    sandbox.space_step = 3.0f;
    sandbox.time_step = 0.5f;
    sandbox.curr_boundary_condition = 1;
}

/**
 * Send the uniforms for this simulation to the compute shader
 * 
 * @param cmap_str String representing a color map to use
 * @param boundary_condition Simulation boundary condition (see Sandbox constructor)
 * @param paused Is the simulation paused?
 * @param dx Space Step
 * @param dt Time Step
 */
void Heat::set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) {
    heatCS.bind();
    heatCS.set_bool("paused", paused);
    heatCS.set_int("width", this->width);
    heatCS.set_int("height", this->height);
    heatCS.set_int("boundary_condition", boundary_condition);
    heatCS.set_float("alpha", this->diffusion);
    heatCS.set_float("dx", dx);
    heatCS.set_float("dt", dt);

    heatCS.set_int("brush_layer", this->brush_layer);
    heatCS.set_int("brush_enabled", this->brush_enabled);
    heatCS.set_int("brush_type", this->brush_type);
    heatCS.set_float("brush_value", this->brush_value);
    heatCS.set_int("x_pos", this->x_pos);
    heatCS.set_int("y_pos", this->y_pos);
    heatCS.set_int("brush_radius", this->brush_radius);
    apply_cmap(heatCS, cmap_str);
}