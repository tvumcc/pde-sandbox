#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "gray_scott.hpp"
#include "sandbox.hpp"

#include <iostream>

GrayScott::GrayScott(int width, int height) 
    : gray_scottCS("shaders/gray_scott.glsl"), Grid(width, height, 2)
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
 * Dispatch the compute shader which solves the equation
 */
void GrayScott::solve() {
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

/**
 * Render the GUI for this specific equation
 */
void GrayScott::gui() {
    ImGui::Text("Feed Rate (a)");
    ImGui::SliderFloat("##a", &a, 0.0, 1.0);
    ImGui::Text("Kill Rate (b)");
    ImGui::SliderFloat("##b", &b, 0.0, 1.0);
    ImGui::Text("Diffusion (D)");
    ImGui::SliderFloat("##D", &D, 0.0, 2.0);
    ImGui::Text("Visible Layer");
    ImGui::Combo("##Visible Layer", &visible_layer, layer_strs.data(), layer_strs.size());
    ImGui::Text("Presets");
    if (ImGui::ListBox("##Preset", &preset, preset_strs.data(), preset_strs.size())) {
        a = presets[std::string(preset_strs[preset])].first;
        b = presets[std::string(preset_strs[preset])].second;
    }
}

/**
 * Reset all simulation specific settings to default
 */
void GrayScott::reset_settings() {
    a =  0.037f;
    b = 0.06f;
    D = 2.0f;
    preset = 3;
    visible_layer = 0;
    space_step = 5.0f;
    time_step = 0.5f;
    boundary_condition = 0;
}

/**
 * Send the uniforms for this simulation to the compute shader
 * 
 * @param cmap_str String representing a color map to use
 * @param paused Is the simulation paused?
 */
void GrayScott::set_uniforms(std::string cmap_str, bool paused) {
    gray_scottCS.bind();
    gray_scottCS.set_bool("paused", paused);
    gray_scottCS.set_int("width", width);
    gray_scottCS.set_int("height", height);
    gray_scottCS.set_int("boundary_condition", boundary_condition);
    gray_scottCS.set_float("a", a);    
    gray_scottCS.set_float("b", b);
    gray_scottCS.set_float("D", D);
    gray_scottCS.set_float("dx", space_step);
    gray_scottCS.set_float("dt", time_step);

    gray_scottCS.set_int("visible_layer", visible_layer);
    gray_scottCS.set_int("brush_enabled", brush_enabled);
    gray_scottCS.set_int("x_pos", x_pos);
    gray_scottCS.set_int("y_pos", y_pos);
    gray_scottCS.set_int("brush_radius", brush_radius);
    apply_cmap(gray_scottCS, cmap_str);
}