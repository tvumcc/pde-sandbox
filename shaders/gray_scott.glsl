#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (r32f, binding = 1) uniform image2D u;
layout (r32f, binding = 2) uniform image2D v;

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
uniform int brush_radius;

// Gray-Scott Reaction Diffusion specific settings
uniform float a;
uniform float b;
uniform float D;

// Color Map Poly 6 Coefficients
uniform vec3 c0, c1, c2, c3, c4, c5, c6;

// 6th Order Polynomial Approximation for Matplotlib Color Maps
vec3 cmap(float t) {
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

// Translate x and y coordinates into a 1D index into an SSBO
int getPosition(int x, int y) {
    return (y * width) + x;
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

// Computes the temporal derivative at a coordinate points in the U grid
float du_dt(int x, int y) {
    float du_dx_0 = (U(x, y) - U(x-1, y)) / dx;
    float du_dx_1 = (U(x+1, y) - U(x, y)) / dx;

    float du_dy_0 = (U(x, y) - U(x, y-1)) / dx;
    float du_dy_1 = (U(x, y+1) - U(x, y)) / dx;

    if (boundary_condition == 1) {
        if (x == 0) du_dx_0 = 0.0;
        if (x == width-1) du_dx_1 = 0.0;
        if (y == 0) du_dy_0 = 0.0;
        if (y == height-1) du_dy_1 = 0.0;
    }

    float d2u_dx2 = (du_dx_1 - du_dx_0) / dx;
    float d2u_dy2 = (du_dy_1 - du_dy_0) / dx;

    return (d2u_dx2 + d2u_dy2) + (pow(U(x, y), 2) * V(x, y)) - ((a + b) * U(x, y));
}

// Computes the temporal derivative at a coordinate points in the V grid
float dv_dt(int x, int y) {
    float dv_dx_0 = (V(x, y) - V(x-1, y)) / dx;
    float dv_dx_1 = (V(x+1, y) - V(x, y)) / dx;

    float dv_dy_0 = (V(x, y) - V(x, y-1)) / dx;
    float dv_dy_1 = (V(x, y+1) - V(x, y)) / dx;

    if (boundary_condition == 1) { // Neumann Boundary Condition
        if (x == 0) dv_dx_0 = 0.0;
        if (x == width-1) dv_dx_1 = 0.0;
        if (y == 0) dv_dy_0 = 0.0;
        if (y == height-1) dv_dy_1 = 0.0;
    }

    float d2v_dx2 = (dv_dx_1 - dv_dx_0) / dx;
    float d2v_dy2 = (dv_dy_1 - dv_dy_0) / dx;

    return D * (d2v_dx2 + d2v_dy2) - (pow(U(x, y), 2) * V(x, y)) + a * (1 - V(x, y));
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);
    int pause = paused ? 0 : 1;

    float du_dt = du_dt(location.x, location.y);
    float dv_dt = dv_dt(location.x, location.y);
    int ratio = int(min(1.0, pow(brush_radius, 2) / (pow(location.x - x_pos, 2) + pow(location.y - y_pos, 2))));

    float luminosity = (1 - brush_enabled * ratio) * (U(location.x, location.y) + du_dt * dt * pause) + (brush_enabled * ratio * brush_value);
    imageStore(u, location, vec4(luminosity));
    imageStore(v, location, vec4(V(location.x, location.y) + dv_dt * dt * pause));
    imageStore(imgOutput, location, vec4(cmap(min(1.0, abs(luminosity * 2.0))), 1.0));
}