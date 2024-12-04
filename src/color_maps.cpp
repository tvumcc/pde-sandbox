#include "color_maps.hpp"

#include <iostream>

using namespace glm;

// Credit to https://www.shadertoy.com/view/Nd3fR2 for the MPL color maps
std::unordered_map<std::string, std::vector<vec3>> cmaps = {
    {"Viridis", {
        vec3(0.274344,0.004462,0.331359),
        vec3(0.108915,1.397291,1.388110),
        vec3(-0.319631,0.243490,0.156419),
        vec3(-4.629188,-5.882803,-19.646115),
        vec3(6.181719,14.388598,57.442181),
        vec3(4.876952,-13.955112,-66.125783),
        vec3(-5.513165,4.709245,26.582180),
    }},
    {"Blues_r", {
        vec3(0.042660,0.186181,0.409512),
        vec3(-0.703712,1.094974,2.049478),
        vec3(7.995725,-0.686110,-4.998203),
        vec3(-24.421963,2.680736,7.532937),
        vec3(47.519089,-4.615112,-5.126531),
        vec3(-46.038418,2.606781,0.685560),
        vec3(16.586546,-0.279280,0.447047),
    }}
};

void apply_cmap(AbstractShader shader, std::string cmap_str) {
    for (int i = 0; i < 7; i++) {
        shader.set_vec3("c" + std::to_string(i), cmaps[cmap_str][i]);
    }
}