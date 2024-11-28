#include <glad/glad.h>

#include "grid.hpp"

#include <algorithm>

Grid::Grid(int width, int height, int num_layers) {
    this->width = width;
    this->height = height;

    // Initialize the output image texture
	glGenTextures(1, &image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, this->width, this->height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Initialize the data SSBOs
    ssbos = std::vector<unsigned int>(num_layers);
    std::vector<float> initial_data = std::vector<float>(width * height, 0.2);
    for (int i = 0; i < ssbos.size(); i++) {
        glGenBuffers(1, &ssbos[i]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, width * height * sizeof(float), initial_data.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ssbos[i]);
    }
}

void Grid::brush(int x_pos, int y_pos, int radius, float value) {
    if (x_pos < 0 || x_pos >= width || y_pos < 0 || y_pos >= height) return;

    int begin = std::max(y_pos - radius, 0) * width;
    int end = std::min(y_pos + radius, height - 1) * width;
    int length = end-begin + 1;

    float* map = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, begin * sizeof(float), length * sizeof(float), GL_MAP_WRITE_BIT);

    for (int y = -radius; y <= radius; y++) {
        int y_offset = (y + y_pos);
        if (y_offset < 0 || y >= height) continue;

        for (int x = -radius; x <= radius; x++) {
            int x_offset = (x + x_pos);
            if (x_offset < 0 || x_offset >= width) continue;

            int offset = y_offset * width + (x + x_pos); 
            if (offset > 0 && offset < width * height && x*x + y*y <= radius * radius) {
                // Cool Gradient:
                // map[offset-begin] = value * (1.0 - (x*x + y*y) / (float)(radius * radius));
                map[offset-begin] = value;
            }
        }
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}