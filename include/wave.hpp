#pragma once
#include "shader.hpp"
#include "grid.hpp"

class Wave : public Grid {
public:
    float diffusion;
    float dx;
    float dy;
    float dt;

    ComputeShader waveCS;

    Wave(int width, int height);

    void solve() override;
    void gui() override;
    void reset_settings() override;
    void set_uniforms(std::string cmap_str, bool paused) override;
};