#pragma once
#include <vector>
#include <string>

class Grid {
public:
    int width;
    int height;
    int brush_layer;
    int unit_idx;

    // Buffer IDs
    unsigned int image;
    std::vector<unsigned int> ssbos;

    Grid(int width = 0, int height = 0, int num_layers = 0, float initial_layer_value = 0.0);

    void brush(int x_pos, int y_pos, int radius, float value);
    void brushGaussian(int x_pos, int y_pos, int radius, float value);
    void resize(int width, int height);
    void clear();
    void set_pixelated(bool pixels);
    void bind();

    virtual void solve() = 0;
    virtual void gui() = 0;
    virtual void reset_settings() = 0;
    virtual void set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) = 0;
};