#pragma once
#include <vector>
#include <string>

class Sandbox;

class Grid {
public:
    // Dimensions
    int width; // The number of cells in the grid along the x-axis
    int height; // The number of cells in the grid along the y-axis

    // Brush
    int brush_enabled; // 1 if the brush is down and 0 if it is up
    int brush_radius; // The radius of the circle created by the brush
    int x_pos, y_pos; // The coordinates of the mouse when the brush is down

    // Simulation Settings
    int resolution; // The number of pixels per cell
    float space_step; // The dx in the Finite Difference Approximation
    float time_step; // The dt in the Finite Difference Approximation
    int boundary_condition; // Specifies the behavior of the solution near the boundaries (Dirichlet, Neumann, or Periodic)

    // Texture IDs
    unsigned int image; // The ID for the output image 2D texture
    std::vector<unsigned int> layers; // The IDs for the 2D textures storing the scalar fields associated with each layer

    Grid(int width = 0, int height = 0, int num_layers = 0);

    void resize(int width, int height);
    void clear();
    void set_pixelated(bool pixels);
    void bind();
    virtual void brush(int x_pos, int y_pos);

    virtual void solve() = 0;
    virtual void gui() = 0;
    virtual void reset_settings() = 0;
    virtual void set_uniforms(std::string cmap_str, bool paused) = 0;
};