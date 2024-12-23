#include <glad/glad.h>

#include "grid.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <algorithm>

/**
 * Constructs a new 2D grid with a given width and height and number of layers. Each layer corresponds to its own SSBO of floats.
 * 
 * @param width The initial width of the image texture and each SSBO
 * @param height The initial height of the image texture and each SSBO
 * @param num_layers Number of SSBOs to intialize and keep track of
 * @param initial_layer_value The initial value to reset all the values of each SSBO to when the grid is reset
 */
Grid::Grid(int width, int height, int num_layers, float initial_layer_value) {
    this->width = width;
    this->height = height;

    this->brush_layer = 0;
    this->brush_enabled = 0;
    this->brush_type = 0;
    this->brush_value = 1.0f;
    this->x_pos = 0;
    this->y_pos = 0;
    this->brush_radius = 10;

    // Initialize the output image texture
	glGenTextures(1, &image);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, this->width, this->height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Initialize the texture for each layer
    std::vector<float> initial_data = std::vector<float>(width * height, initial_layer_value);
    layers = std::vector<unsigned int>(num_layers);
    for (int i = 0; i < layers.size(); i++) {
        glGenTextures(1, &layers[i]);
        glBindTexture(GL_TEXTURE_2D, layers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, this->width, this->height, 0, GL_RED, GL_FLOAT, 0);
        glBindImageTexture(i+1, layers[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    }
}

/**
 * Resizes all SSBOs and the image texture to the specified dimensions
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
 * Clears the image texture and all SSBOs
 */
void Grid::clear() {
    glBindTexture(GL_TEXTURE_2D, image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    for (int i = 0; i < layers.size(); i++) {
        std::vector<float> initial_data = std::vector<float>(width * height, 0.0);

        glGenTextures(1, &layers[i]);
        glBindTexture(GL_TEXTURE_2D, layers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, this->width, this->height, 0, GL_RED, GL_FLOAT, initial_data.data());
        glBindImageTexture(i+1, layers[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    }
}

/**
 * Changes the magnification filter of the image texture.
 * In effect, GL_NEAREST will make the image look pixelated while GL_LINEAR will smoothen it out.
 * 
 * @param pixels If true, image becomes pixelated and smooth otherwise
 */
void Grid::set_pixelated(bool pixels) {
    glBindTexture(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixels ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

/**
 * Binds the image texture and all SSBOs to the current OpenGL state
 */
void Grid::bind() {
    glBindTexture(GL_TEXTURE_2D, this->image);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    for (int i = 0; i < layers.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, layers[i]);
        glBindImageTexture(i+1, layers[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    }
}