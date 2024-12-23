#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "wave.hpp"
#include "sandbox.hpp"

#include <iostream>

Wave::Wave(int width, int height) 
    : waveCS("shaders/wave.glsl"), Grid(width, height, 2, 0.0f)
{
    reset_settings();
}

/**
 * Dispatch the compute shader which solves the equation
 */
void Wave::solve() {
    glDispatchCompute(this->width, this->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for the Wave Equation Simulation using ImGui
 */
void Wave::gui() {}

/**
 * Reset all simulation specific settings to default
 */
void Wave::reset_settings() {
    this->brush_radius = 3;
}

/**
 * 
 */
void Wave::use_recommended_settings(Sandbox& sandbox) {
    sandbox.resolution = 2;
    sandbox.space_step = 0.1f;
    sandbox.time_step = 0.01f;
    sandbox.curr_boundary_condition = 1;
    sandbox.resize(sandbox.window_width, sandbox.window_height);
}

/**
 * Send the uniforms for this simulation to the compute shader
 */
void Wave::set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) {
    waveCS.bind();
    waveCS.set_bool("paused", paused);
    waveCS.set_int("width", this->width);
    waveCS.set_int("height", this->height);
    waveCS.set_int("boundary_condition", boundary_condition);
    waveCS.set_float("dx", dx);
    waveCS.set_float("dt", dt);

    waveCS.set_int("brush_layer", this->brush_layer);
    waveCS.set_int("brush_enabled", this->brush_enabled);
    waveCS.set_int("brush_type", this->brush_type);
    waveCS.set_float("brush_value", this->brush_value);
    waveCS.set_int("x_pos", this->x_pos);
    waveCS.set_int("y_pos", this->y_pos);
    waveCS.set_int("brush_radius", this->brush_radius);
    apply_cmap(waveCS, cmap_str);
}