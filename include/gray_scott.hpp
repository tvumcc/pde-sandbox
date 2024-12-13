#pragma once
#include "shader.hpp"
#include "grid.hpp"

class GrayScott : public Grid {
public:
    float a;
    float b;
    float D;

    ComputeShader gray_scottCS;

    GrayScott(int width, int height);

    void solve() override;
    void gui() override;
    void reset_settings() override;
    void set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) override;
};