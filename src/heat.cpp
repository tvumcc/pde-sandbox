#include <glad/glad.h>
#include <imgui/imgui.h>

#include "color_maps.hpp"
#include "heat.hpp"

#include <iostream>

Heat::Heat(int width, int height) 
    : heatCS("shaders/heat.glsl"), Grid(width, height, 1, 0.0f)
{
    this->dt = 0.05;
    this->dx = 1.0;
    this->dy = 1.0;
    this->diffusion = 1.0;
}

void Heat::solve() {
    glDispatchCompute(this->width, this->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void Heat::gui() {
    ImGui::Text("Diffusion");
    ImGui::SliderFloat("##Diffusion", &diffusion, 0.01, 3.0);
}

void Heat::set_uniforms(std::string cmap_str, bool paused) {
    heatCS.bind();
    heatCS.set_bool("paused", paused);
    heatCS.set_int("width", this->width);
    heatCS.set_int("height", this->height);
    heatCS.set_float("alpha", this->diffusion);
    apply_cmap(heatCS, cmap_str);
}