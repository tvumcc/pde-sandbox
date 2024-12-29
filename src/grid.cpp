#include <glad/glad.h>

#include "grid.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <algorithm>

/**
 * Constructs a new 2D grid with a given width and height and number of layers. Each layer corresponds to a 2D R32F texture.
 * 
 * @param width The initial width of all textures (output image and all layer textures)
 * @param height The initial height of all textures (output image and all layer textures)
 * @param num_layers Number of textures to intialize and keep track of
 */
Grid::Grid(int width, int height, int num_layers) {
    this->width = width;
    this->height = height;

    // Default settings for each grid
    brush_enabled = 0;
    x_pos = 0;
    y_pos = 0;
    space_step = 3.0f;
    time_step = 0.1f;
    brush_radius = 10;
    resolution = 8;
    pixelated = false;

    // Initialize the output image texture
	glGenTextures(1, &image);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixelated ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Initialize the texture for each layer
    std::vector<float> initial_data = std::vector<float>(width * height, 0.0);
    layers = std::vector<unsigned int>(num_layers);
    for (int i = 0; i < layers.size(); i++) {
        glGenTextures(1, &layers[i]);
        glBindTexture(GL_TEXTURE_2D, layers[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
        glBindImageTexture(i+1, layers[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    }
}

/**
 * Resizes all textures to the specified dimensions
 * 
 * @param width Width to resize the grids to
 * @param height Height to resize the grids to
 */
void Grid::resize(int width, int height) {
    this->width = width;
    this->height = height;
    clear();
}

/**
 * Clears all textures to 0
 */
void Grid::clear() {
    glGenTextures(1, &image);
    glBindTexture(GL_TEXTURE_2D, image);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixelated ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    for (int i = 0; i < layers.size(); i++) {
        glGenTextures(1, &layers[i]);
        glBindTexture(GL_TEXTURE_2D, layers[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
        glBindImageTexture(i+1, layers[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    }
}

/**
 * Changes the magnification filter of the image texture.
 * In effect, GL_NEAREST will make the image look pixelated while GL_LINEAR will smoothen it out.
 */
void Grid::set_pixelated() {
    glBindTexture(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixelated ? GL_NEAREST : GL_LINEAR);
}

/**
 * Binds all textures to the current OpenGL state
 */
void Grid::bind() {
    glBindTexture(GL_TEXTURE_2D, image);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    for (int i = 0; i < layers.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, layers[i]);
        glBindImageTexture(i+1, layers[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    }
}

/**
 * Updates the currently tracked mouse position and brush enabled status
 * 
 * @param x_pos The x coordinate of the mouse mapped to the grid, not the window
 * @param y_pos The y coordinate of the mouse mapped to the grid, not the window
 */
void Grid::brush(int x_pos, int y_pos) {
    this->x_pos = x_pos;
    this->y_pos = y_pos;

    // Only enable brush if the mouse coordinates reside within the grid
    brush_enabled = x_pos >= 0 && x_pos < width && y_pos >= 0 && y_pos < height;
}