#pragma once
#include <vector>
#include <string>

class Grid {
public:
    int width;
    int height;
    int unit_idx;

    // Brush
    int brush_layer;
    int brush_enabled;
    int brush_type;
    float brush_value;
    int brush_radius;
    int x_pos, y_pos;

    // Buffer IDs
    unsigned int image;
    std::vector<unsigned int> layers;

    Grid(int width = 0, int height = 0, int num_layers = 0, float initial_layer_value = 0.0);

    void resize(int width, int height);
    void clear();
    void set_pixelated(bool pixels);
    void bind();

    virtual void solve() = 0;
    virtual void gui() = 0;
    virtual void reset_settings() = 0;
    virtual void set_uniforms(std::string cmap_str, int boundary_condition, bool paused, float dx, float dt) = 0;
};