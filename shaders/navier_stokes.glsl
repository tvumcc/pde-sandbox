#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (r32f, binding = 1) uniform image2D u;
layout (r32f, binding = 2) uniform image2D v;
layout (r32f, binding = 3) uniform image2D p;
layout (r32f, binding = 4) uniform image2D s;

// Dimensions of the grids
uniform int width;
uniform int height;

// PDE settings
uniform bool paused;
uniform int boundary_condition;
uniform float dx;
uniform float dt;

// Brush settings
uniform int brush_layer;
uniform int brush_enabled;
uniform int brush_type;
uniform float brush_value;
uniform int x_pos;
uniform int y_pos;
uniform int prev_x_pos;
uniform int prev_y_pos;
uniform int brush_radius;
uniform int visible_layer;

// Navier-Stokes Reaction Diffusion specific settings
uniform float viscosity;

// Color Map Poly 6 Coefficients
uniform vec3 c0, c1, c2, c3, c4, c5, c6;

// 6th Order Polynomial Approximation for Matplotlib Color Maps
vec3 cmap(float t) {
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

// Performs the remainder operation
int fmod(int x, int y) {
    return (x % y + y) % y;
}

// Accesses the value of the U grid at a coordinate while respecting value-based boundary conditions
float U(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 2) { // Periodic Boundary Condition
            return imageLoad(u, ivec2(fmod(x, width), fmod(y, height))).r;
        } else { // Dirichlet Boundary Condition
            return 0.0;
        }
    }

    return imageLoad(u, ivec2(x, y)).r;
}

// Accesses the value of the V grid at a coordinate while respecting value-based boundary conditions
float V(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 2) { // Periodic Boundary Condition
            return imageLoad(v, ivec2(fmod(x, width), fmod(y, height))).r;
        } else { // Dirichlet Boundary Condition
            return 0.0;
        }
    }

    return imageLoad(v, ivec2(x, y)).r;
}

// Accesses the value of the P grid at a coordinate while respecting value-based boundary conditions
float P(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 2) { // Periodic Boundary Condition
            return imageLoad(p, ivec2(fmod(x, width), fmod(y, height))).r;
        } else { // Dirichlet Boundary Condition
            return 0.0;
        }
    }

    return imageLoad(p, ivec2(x, y)).r;
}

// Accesses the value of the S grid at a coordinate while respecting value-based boundary conditions
float S(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (x < 0) {
            return 1.0;
        } else if (x >= width) {
            return 0.0;
        } else {
            return imageLoad(s, ivec2(fmod(x, width), fmod(y, height))).r;
        }
    }

    return imageLoad(s, ivec2(x, y)).r;
}

// Computes the temporal derivative at a coordinate point in the U grid
float du_dt(int x, int y) {
    float du_dx_0 = (U(x, y) - U(x-1, y)) / dx;
    float du_dx_1 = (U(x+1, y) - U(x, y)) / dx;

    float du_dy_0 = (U(x, y) - U(x, y-1)) / dx;
    float du_dy_1 = (U(x, y+1) - U(x, y)) / dx;

    float dp_dx = (P(x+1, y) - P(x, y)) / dx;

    if (boundary_condition == 1) {
        if (x == 0) {
            du_dx_0 = 0.0;
            dp_dx = 0.0;
        }
        if (x == width-1) {
            du_dx_1 = 0.0;
            dp_dx = 0.0;
        }
        if (y == 0) du_dy_0 = 0.0;
        if (y == height-1) du_dy_1 = 0.0;
    }

    float d2u_dx2 = (du_dx_1 - du_dx_0) / dx;
    float d2u_dy2 = (du_dy_1 - du_dy_0) / dx;

    return viscosity * (d2u_dx2 + d2u_dy2) - (U(x, y) * (du_dx_0 + du_dx_1) * 0.5 + V(x, y) * (du_dy_0 + du_dy_1) * 0.5) - dp_dx;
}

// Computes the temporal derivative at a coordinate point in the V grid
float dv_dt(int x, int y) {
    float dv_dx_0 = (V(x, y) - V(x-1, y)) / dx;
    float dv_dx_1 = (V(x+1, y) - V(x, y)) / dx;

    float dv_dy_0 = (V(x, y) - V(x, y-1)) / dx;
    float dv_dy_1 = (V(x, y+1) - V(x, y)) / dx;

    float dp_dy_0 = (P(x, y) - P(x, y-1)) / dx;
    float dp_dy_1 = (P(x, y+1) - P(x, y)) / dx;
    float dp_dy = (dp_dy_0 + dp_dy_1) * 0.5;

    if (boundary_condition == 1) { // Neumann Boundary Condition
        if (x == 0) dv_dx_0 = 0.0;
        if (x == width-1) dv_dx_1 = 0.0;
        if (y == 0) {
            dv_dy_0 = 0.0;
            dp_dy = 0.0;
        }
        if (y == height-1) {
            dv_dy_1 = 0.0;
            dp_dy = 0.0;
        }
    }

    float d2v_dx2 = (dv_dx_1 - dv_dx_0) / dx;
    float d2v_dy2 = (dv_dy_1 - dv_dy_0) / dx;

    return viscosity * (d2v_dx2 + d2v_dy2) - (U(x, y) * (dv_dx_0 + dv_dx_1) * 0.5 + V(x, y) * (dv_dx_0 + dv_dy_1) * 0.5) - dp_dy;
}

// Computes the temporal derivative at a coordinate point in the P grid
float dp_dt(int x, int y) {
    float dp_dx_0 = (P(x, y) - P(x-1, y)) / dx;
    float dp_dx_1 = (P(x+1, y) - P(x, y)) / dx;

    float dp_dy_0 = (P(x, y) - P(x, y-1)) / dx;
    float dp_dy_1 = (P(x, y+1) - P(x, y)) / dx;

    float du_dx = (U(x+1, y) - U(x, y)) / dx;
    float dv_dy = (V(x, y+1) - V(x, y)) / dx;

    if (true) { // Neumann Boundary Condition
        if (x == 0) {
            dp_dx_0 = 0.0;
            du_dx = 0.0;
        }
        if (x == width-1) {
            dp_dx_1 = 0.0;
            du_dx = 0.0;
        }
        if (y == 0) {
            dp_dy_0 = 0.0;
            dv_dy = 0.0;
        }
        if (y == height-1) {
            dp_dy_1 = 0.0;
            dv_dy = 0.0;
        }
    }

    float d2p_dx2 = (dp_dx_1 - dp_dx_0) / dx;
    float d2p_dy2 = (dp_dy_1 - dp_dy_0) / dx;
    float M = 0.5;

    return viscosity * (d2p_dx2 + d2p_dy2) - (1.0 / (M * M)) * (du_dx + dv_dy);
}

// Computes the temporal derivative at a coordinate point in the S grid
float ds_dt(int x, int y) {
    float ds_dx_0 = (S(x, y) - S(x-1, y)) / dx;
    float ds_dx_1 = (S(x+1, y) - S(x, y)) / dx;

    float ds_dy_0 = (S(x, y) - S(x, y-1)) / dx;
    float ds_dy_1 = (S(x, y+1) - S(x, y)) / dx;

    if (false) { // Neumann Boundary Condition
        if (x == 0) ds_dx_0 = 0.0;
        if (x == width-1) ds_dx_1 = 0.0;
        if (y == 0) ds_dy_0 = 0.0;
        if (y == height-1) ds_dy_1 = 0.0;
    }

    float ds_dx = (ds_dx_0 + ds_dx_1) * 0.5;
    float ds_dy = (ds_dy_0 + ds_dy_1) * 0.5;

    float d2s_dx2 = (ds_dx_1 - ds_dx_0) / dx;
    float d2s_dy2 = (ds_dy_1 - ds_dy_0) / dx;

    return 0.05 * (d2s_dx2 + d2s_dy2) - (U(x, y) * ds_dx + V(x, y) * ds_dy);
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);
    int ratio = int(min(1.0, pow(brush_radius, 2) / (pow(location.x - x_pos, 2) + pow(location.y - y_pos, 2))));
    int pause = paused ? 0 : 1;

    float du_dt = du_dt(location.x, location.y);
    float dv_dt = dv_dt(location.x, location.y);
    float dp_dt = dp_dt(location.x, location.y);
    float ds_dt = ds_dt(location.x, location.y);

    if (brush_layer == 0) {
        if (brush_enabled == 1 && prev_x_pos >= 0 && prev_y_pos >= 0 && !(prev_x_pos == x_pos || prev_y_pos == y_pos)) {
            int ratio = int(min(1.0, pow(brush_radius, 2) / (pow(location.x - prev_x_pos, 2) + pow(location.y - prev_y_pos, 2))));
            vec2 normal = 2.0 * normalize(vec2(float(x_pos - prev_x_pos), float(y_pos - prev_y_pos)));

            imageStore(u, location, vec4((1 - brush_enabled * ratio) * (U(location.x, location.y) + du_dt * dt * pause) + (brush_enabled * ratio * normal.x)));
            imageStore(v, location, vec4((1 - brush_enabled * ratio) * (V(location.x, location.y) + dv_dt * dt * pause) + (brush_enabled * ratio * normal.y)));
            imageStore(p, location, vec4(P(location.x, location.y) + dp_dt * dt * pause));
            imageStore(s, location, vec4(S(location.x, location.y) + ds_dt * dt * pause));
        } else {
            imageStore(u, location, vec4(U(location.x, location.y) + du_dt * dt * pause));
            imageStore(v, location, vec4(V(location.x, location.y) + dv_dt * dt * pause));
            imageStore(p, location, vec4(P(location.x, location.y) + dp_dt * dt * pause));
            imageStore(s, location, vec4(S(location.x, location.y) + ds_dt * dt * pause));
        }
    } else if (brush_layer == 1) {
        imageStore(u, location, vec4(U(location.x, location.y) + du_dt * dt * pause));
        imageStore(v, location, vec4(V(location.x, location.y) + dv_dt * dt * pause));
        imageStore(p, location, vec4(P(location.x, location.y) + dp_dt * dt * pause));
        imageStore(s, location, vec4((1 - brush_enabled * ratio) * (S(location.x, location.y) + ds_dt * dt * pause) + (brush_enabled * ratio * brush_value)));
    }


    if (visible_layer == 0) {
        imageStore(imgOutput, location, vec4(cmap(min(1.0, abs(U(location.x, location.y)))), 1.0));
    } else if (visible_layer == 1) {
        imageStore(imgOutput, location, vec4(cmap(min(1.0, abs(V(location.x, location.y)))), 1.0));
    } else if (visible_layer == 2) {
        float magnitude = sqrt(pow(U(location.x, location.y), 2) + pow(V(location.x, location.y), 2));
        imageStore(imgOutput, location, vec4(cmap(min(1.0, magnitude)), 1.0));
    } else if (visible_layer == 3) {
        imageStore(imgOutput, location, vec4(cmap(min(1.0, abs(S(location.x, location.y)))), 1.0));
    }

}