#pragma once
#include "grid.hpp"

#include <vector>
#include <memory>

class Sandbox {
public:
    int window_width;
    int window_height;
    int gui_width;

    int resolution;
    int brush_radius;
    float space_step;
    float time_step;
    bool pixels;
    bool paused;

    // Dropdown UI Elements
    std::vector<const char*> cmap_strs;
    std::vector<const char*> sim_strs;
    std::vector<const char*> boundary_condition_strs;
    std::vector<const char*> brush_strs;
    int curr_cmap;
    int curr_sim;
    int curr_boundary_condition;
    int curr_brush;

    std::vector<std::shared_ptr<Grid>> grids;
    std::shared_ptr<Grid> grid;

    Sandbox(int width, int height);

    void render_gui();
    void resize(int width, int height);
    void advance_step();
    void bind_current_grid();
    void brush(double x_pos, double y_pos);
private:
    void reset_settings();
    void reset_grid();
};

enum class Brush {
    Circle = 0,
    Gaussian
};