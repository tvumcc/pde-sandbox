#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "navier_stokes.hpp"

#include <iostream>

NavierStokes::NavierStokes(int width, int height) 
    : navier_stokesCS("shaders/navier_stokes.glsl"), Grid(width, height, 4, 0.0f)
{
    brush_layer = 0;

    visible_layer_strs.resize(4);
    visible_layer_strs = {"Velocity (x)", "Velocity (y)", "Velocity (Magnitude)", "Dye"};
    brush_layer_strs.resize(2);
    brush_layer_strs = {"Velocity", "Dye"};

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
    ImGui::Text("Visible Layer");
    ImGui::Combo("##Visible Layer", &visible_layer, visible_layer_strs.data(), visible_layer_strs.size());
    ImGui::Text("Brush Layer");
    ImGui::Combo("##Brush Layer", &brush_layer, brush_layer_strs.data(), brush_layer_strs.size());
}

/**
 * Reset all simulation specific settings to default
 */
void NavierStokes::reset_settings() {
    this->viscosity =  1.0f;
    this->visible_layer = 3;
    this->brush_layer = 0;
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

    navier_stokesCS.set_int("visible_layer", this->visible_layer);
    navier_stokesCS.set_int("brush_layer", this->brush_layer);
    navier_stokesCS.set_int("brush_enabled", this->brush_enabled);
    navier_stokesCS.set_int("brush_type", this->brush_type);
    navier_stokesCS.set_float("brush_value", this->brush_value);
    navier_stokesCS.set_int("x_pos", this->x_pos);
    navier_stokesCS.set_int("y_pos", this->y_pos);
    navier_stokesCS.set_int("brush_radius", this->brush_radius);
    apply_cmap(navier_stokesCS, cmap_str);
}