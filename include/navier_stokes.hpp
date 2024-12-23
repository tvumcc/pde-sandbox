#pragma once
#include "shader.hpp"
#include "grid.hpp"

#include <map>

class NavierStokes : public Grid {
public:
    float viscosity;

    int visible_layer;
    std::vector<const char*> visible_layer_strs;

    ComputeShader navier_stokesCS;

    NavierStokes(int width, int height);

    void solve() override;
    void gui() override;
    void reset_settings() override;
    void set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) override;
};