#pragma once
#include <glm/glm.hpp>

#include "shader.hpp"

#include <vector>
#include <unordered_map>
#include <string>

extern std::unordered_map<std::string, std::vector<glm::vec3>> cmaps;

void apply_cmap(AbstractShader shader, std::string cmap_str);