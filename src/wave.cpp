#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "wave.hpp"
#include "sandbox.hpp"

#include <iostream>

Wave::Wave(int width, int height) 
    : waveCS("shaders/wave.glsl"), Grid(width, height, 2)
{
    reset_settings();
}

/**
 * Dispatch the compute shader which solves the equation
 */
void Wave::solve() {
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for this specific equation
 */
void Wave::gui() {}

/**
 * Reset all simulation specific settings to default
 */
void Wave::reset_settings() {
    brush_radius = 3;
    space_step = 0.1f;
    time_step = 0.01f;
    boundary_condition = 1;
}

/**
 * Send the uniforms for this simulation to the compute shader
 * 
 * @param cmap_str String representing a color map to use
 * @param paused Is the simulation paused?
 */
void Wave::set_uniforms(std::string cmap_str, bool paused) {
    waveCS.bind();
    waveCS.set_bool("paused", paused);
    waveCS.set_int("width", width);
    waveCS.set_int("height", height);
    waveCS.set_int("boundary_condition", boundary_condition);
    waveCS.set_float("dx", space_step);
    waveCS.set_float("dt", time_step);

    waveCS.set_int("brush_enabled", brush_enabled);
    waveCS.set_int("x_pos", x_pos);
    waveCS.set_int("y_pos", y_pos);
    waveCS.set_int("brush_radius", brush_radius);
    apply_cmap(waveCS, cmap_str);
}