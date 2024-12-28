#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "navier_stokes.hpp"
#include "sandbox.hpp"

#include <iostream>

NavierStokes::NavierStokes(int width, int height) 
    : navier_stokesCS("shaders/navier_stokes.glsl"), Grid(width, height, 4)
{
    brush_layer = 0;
    prev_x_pos = -1;
    prev_y_pos = -1;

    visible_layer_strs.resize(4);
    visible_layer_strs = {"Velocity (x)", "Velocity (y)", "Velocity (Magnitude)", "Dye"};
    brush_layer_strs.resize(2);
    brush_layer_strs = {"Velocity", "Dye"};

    reset_settings();
}

/**
 * Updates the currently tracked mouse position and brush enabled status, while also keeping track of the previous mouse position
 * 
 * @param x_pos The x coordinate of the mouse mapped to the grid, not the window
 * @param y_pos The y coordinate of the mouse mapped to the grid, not the window
 */
void NavierStokes::brush(int x_pos, int y_pos) {
    if (brush_enabled && x_pos >= 0 && x_pos < this->width && y_pos >= 0 && y_pos < this->height) {
        prev_x_pos = this->x_pos;
        prev_y_pos = this->y_pos;
    } else {
        prev_x_pos = -1;
        prev_y_pos = -1;
    }

    this->x_pos = x_pos;
    this->y_pos = y_pos;
    brush_enabled = x_pos >= 0 && x_pos < width && y_pos >= 0 && y_pos < height;
}

/**
 * Dispatch the compute shader which solves the equation
 */
void NavierStokes::solve() {
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for this specific equation
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
    viscosity =  1.0f;
    visible_layer = 3;
    brush_layer = 0;
    space_step = 0.5f;
    time_step = 0.03f;
    boundary_condition = 0;
}

/**
 * Send the uniforms for this simulation to the compute shader
 * 
 * @param cmap_str String representing a color map to use
 * @param paused Is the simulation paused?
 */
void NavierStokes::set_uniforms(std::string cmap_str, bool paused) {
    navier_stokesCS.bind();
    navier_stokesCS.set_bool("paused", paused);
    navier_stokesCS.set_int("width", width);
    navier_stokesCS.set_int("height", height);
    navier_stokesCS.set_int("boundary_condition", boundary_condition);
    navier_stokesCS.set_float("viscosity", viscosity);
    navier_stokesCS.set_float("dx", space_step);
    navier_stokesCS.set_float("dt", time_step);

    navier_stokesCS.set_int("visible_layer", visible_layer);
    navier_stokesCS.set_int("brush_layer", brush_layer);
    navier_stokesCS.set_int("brush_enabled", brush_enabled);
    navier_stokesCS.set_int("x_pos", x_pos);
    navier_stokesCS.set_int("y_pos", y_pos);
    navier_stokesCS.set_int("prev_x_pos", prev_x_pos);
    navier_stokesCS.set_int("prev_y_pos", prev_y_pos);
    navier_stokesCS.set_int("brush_radius", brush_radius);
    apply_cmap(navier_stokesCS, cmap_str);
}