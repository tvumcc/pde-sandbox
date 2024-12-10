#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "wave.hpp"

#include <iostream>

Wave::Wave(int width, int height) 
    : waveCS("shaders/wave.glsl"), Grid(width, height, 2, 0.0f)
{
    this->dt = 0.05;
    this->dx = 1.0;
    this->dy = 1.0;
    this->diffusion = 1.0;
}

/**
 * Dispath the compute shader which solves the equation
 */
void Wave::solve() {
    glDispatchCompute(this->width, this->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

/**
 * Render the GUI for the Heat Equation Simulation using ImGui
 */
void Wave::gui() {
    // ImGui::Text("Diffusion");
    // ImGui::SliderFloat("##Diffusion", &diffusion, 0.01, 3.0);
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
    waveCS.set_float("alpha", this->diffusion);
    waveCS.set_float("dx", dx);
    waveCS.set_float("dt", dt);
    apply_cmap(waveCS, cmap_str);
}