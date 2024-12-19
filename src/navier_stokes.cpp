#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "navier_stokes.hpp"

#include <iostream>

NavierStokes::NavierStokes(int width, int height) 
    : navier_stokesCS("shaders/navier_stokes.glsl"), Grid(width, height, 4, 0.0f)
{
    brush_layer = 0;
    reset_settings();
}

/**
 * Dispath the compute shader which solves the equation
 */
void NavierStokes::solve() {
    glDispatchCompute(this->width, this->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for the Heat Equation Simulation using ImGui
 */
void NavierStokes::gui() {
    ImGui::Text("Viscosity (v)");
    ImGui::SliderFloat("##Viscosity", &viscosity, 0.0, 1.0);
}

/**
 * Reset all simulation specific settings to default
 */
void NavierStokes::reset_settings() {
    this->viscosity =  1.0f;
}

/**
 * Send the uniforms for this simulation to the compute shader
 */
void NavierStokes::set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) {
    navier_stokesCS.bind();
    navier_stokesCS.set_bool("paused", paused);
    navier_stokesCS.set_int("width", this->width);
    navier_stokesCS.set_int("height", this->height);
    navier_stokesCS.set_int("boundary_condition", boundary_condition);
    navier_stokesCS.set_float("viscosity", this->viscosity);
    navier_stokesCS.set_float("dx", dx);
    navier_stokesCS.set_float("dt", dt);
    apply_cmap(navier_stokesCS, cmap_str);
}