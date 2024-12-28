#pragma once
#include "shader.hpp"
#include "grid.hpp"

#include <map>

class NavierStokes : public Grid {
public:
    float viscosity;
    int prev_x_pos;
    int prev_y_pos;
    int brush_layer;

    int visible_layer;
    std::vector<const char*> visible_layer_strs;
    std::vector<const char*> brush_layer_strs;

    ComputeShader navier_stokesCS;

    NavierStokes(int width, int height);

    void brush(int x_pos, int y_pos) override;
    void solve() override;
    void gui() override;
    void reset_settings() override;
    void set_uniforms(std::string cmap_str, bool paused) override;
};