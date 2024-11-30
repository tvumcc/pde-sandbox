#pragma once
#include "shader.hpp"
#include "grid.hpp"

class Heat : public Grid {
public:
    float diffusion;
    float dx;
    float dy;
    float dt;

    ComputeShader heatCS;

    Heat(int width, int height);
    void solve() override;
    void gui() override;
};