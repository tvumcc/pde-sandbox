#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "gray_scott.hpp"

#include <iostream>

GrayScott::GrayScott(int width, int height) 
    : gray_scottCS("shaders/gray_scott.glsl"), Grid(width, height, 2, 0.0f)
{
    reset_settings();
}

/**
 * Dispath the compute shader which solves the equation
 */
void GrayScott::solve() {
    glDispatchCompute(this->width, this->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

/**
 * Render the GUI for the Heat Equation Simulation using ImGui
 */
void GrayScott::gui() {
    ImGui::Text("a");
    ImGui::SliderFloat("##a", &a, 0.0, 1.0);
    ImGui::Text("b");
    ImGui::SliderFloat("##b", &b, 0.0, 1.0);
    ImGui::Text("D");
    ImGui::SliderFloat("##D", &D, 0.0, 2.0);
}

/**
 * Reset all simulation specific settings to default
 */
void GrayScott::reset_settings() {
    this->a =  0.037f;
    this->b = 0.06f;
    this->D = 2.0f;
}

/**
 * Send the uniforms for this simulation to the compute shader
 */
void GrayScott::set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) {
    gray_scottCS.bind();
    gray_scottCS.set_bool("paused", paused);
    gray_scottCS.set_int("width", this->width);
    gray_scottCS.set_int("height", this->height);
    gray_scottCS.set_int("boundary_condition", boundary_condition);
    gray_scottCS.set_float("a", this->a);    
    gray_scottCS.set_float("b", this->b);
    gray_scottCS.set_float("D", this->D);
    gray_scottCS.set_float("dx", dx);
    gray_scottCS.set_float("dt", dt);
    apply_cmap(gray_scottCS, cmap_str);
}