#pragma once
#include "grid.hpp"

#include <vector>
#include <memory>

class Sandbox {
public:
    int window_width; // Width of the application window
    int window_height; // Height of the application window
    int gui_width; // Width of the sidebar UI

    // Visual Settings
    bool pixelated; // Determines whether the grid's output image looks pixelated or not
    bool paused; // True if the simulation is paused and false otherwise
    int cmap; // Index of the currently selected color map
    int sim; // Index of the currently selected simulation

    // Dropdown lists for the sidebar UI, see sandbox.cpp for the values in each list
    std::vector<const char*> cmap_strs;
    std::vector<const char*> sim_strs;
    std::vector<const char*> boundary_condition_strs;

    std::vector<std::shared_ptr<Grid>> grids; // Stores pointers to grids representing each simulation of a PDE

    Sandbox(int window_width, int window_height, int gui_width);

    void render_gui();
    void resize(int window_width, int window_height, int gui_width);
    void advance_step();
    void bind_current_grid();
    void brush(double x_pos, double y_pos);
    void reset_settings();
    void reset_grid();
};