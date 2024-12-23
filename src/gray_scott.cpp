#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "gray_scott.hpp"
#include "sandbox.hpp"

#include <iostream>

GrayScott::GrayScott(int width, int height) 
    : gray_scottCS("shaders/gray_scott.glsl"), Grid(width, height, 2, 0.0f)
{
    // See https://visualpde.com/nonlinear-physics/gray-scott/
    presets = {
        {"Labyrinthine", {0.037f, 0.06f}},
        {"Spots", {0.03f, 0.062f}},
        {"Pulsating Spots", {0.025f, 0.06f}},
        {"Worms", {0.078f, 0.061f}},
        {"Holes", {0.039f, 0.058f}},
        {"Spatiotemporal chaos", {0.026f, 0.051f}},
        {"Intermittent chaos/holes", {0.034f, 0.056f}},
        {"Moving Spots", {0.014f, 0.054f}},
        {"Small waves", {0.018f, 0.051f}},
        {"Big waves", {0.014f, 0.045f}},
        {"U-skate world", {0.062f, 0.061f}}
    };

    for (const auto& kp : presets) preset_strs.push_back(kp.first.c_str());

    layer_strs.resize(2);
    layer_strs = {"Chemical A", "Chemical B"};

    reset_settings();
}

/**
 * Dispath the compute shader which solves the equation
 */
void GrayScott::solve() {
    glDispatchCompute(this->width, this->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for the Heat Equation Simulation using ImGui
 */
void GrayScott::gui() {
    ImGui::Text("Feed Rate (a)");
    ImGui::SliderFloat("##a", &a, 0.0, 1.0);
    ImGui::Text("Kill Rate (b)");
    ImGui::SliderFloat("##b", &b, 0.0, 1.0);
    ImGui::Text("Diffusion (D)");
    ImGui::SliderFloat("##D", &D, 0.0, 2.0);
    ImGui::Text("Visible Layer");
    ImGui::Combo("##Visible Layer", &curr_layer, layer_strs.data(), layer_strs.size());
    ImGui::Text("Presets");
    if (ImGui::ListBox("##Preset", &curr_preset, preset_strs.data(), preset_strs.size())) {
        a = presets[std::string(preset_strs[curr_preset])].first;
        b = presets[std::string(preset_strs[curr_preset])].second;
    }
}

/**
 * Reset all simulation specific settings to default
 */
void GrayScott::reset_settings() {
    this->a =  0.037f;
    this->b = 0.06f;
    this->D = 2.0f;
    curr_preset = 3;
    curr_layer = 0;
}

/**
 * 
 */
void GrayScott::use_recommended_settings(Sandbox& sandbox) {
    sandbox.space_step = 5.0f;
    sandbox.time_step = 0.5f;
    sandbox.curr_boundary_condition = 0;
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

    gray_scottCS.set_int("visible_layer", this->curr_layer);
    gray_scottCS.set_int("brush_layer", this->brush_layer);
    gray_scottCS.set_int("brush_enabled", this->brush_enabled);
    gray_scottCS.set_int("brush_type", this->brush_type);
    gray_scottCS.set_float("brush_value", this->brush_value);
    gray_scottCS.set_int("x_pos", this->x_pos);
    gray_scottCS.set_int("y_pos", this->y_pos);
    gray_scottCS.set_int("brush_radius", this->brush_radius);
    apply_cmap(gray_scottCS, cmap_str);
}