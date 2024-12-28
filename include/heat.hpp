#pragma once
#include "shader.hpp"
#include "grid.hpp"

class Heat : public Grid {
public:
    float diffusion;

    ComputeShader heatCS;

    Heat(int width, int height);

    void solve() override;
    void gui() override;
    void reset_settings() override;
    void set_uniforms(std::string cmap_str, bool paused) override;
};