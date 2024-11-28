#pragma once
#include <vector>

class Grid {
public:
    int width;
    int height;

    // Buffer IDs
    unsigned int image;
    std::vector<unsigned int> ssbos;

    Grid(int width, int height, int num_layers);

    void brush(int x_pos, int y_pos, int radius, float value);
};