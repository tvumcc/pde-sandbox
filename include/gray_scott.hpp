#pragma once
#include "shader.hpp"
#include "grid.hpp"

#include <map>

class GrayScott : public Grid {
public:
    float a;
    float b;
    float D;

    std::vector<const char*> layer_strs;
    int visible_layer;

    std::map<std::string, std::pair<float, float>> presets;
    std::vector<const char*> preset_strs;
    int preset;

    ComputeShader gray_scottCS;

    GrayScott(int width, int height);

    void solve() override;
    void gui() override;
    void reset_settings() override;
    void set_uniforms(std::string cmap_str, bool paused) override;
};