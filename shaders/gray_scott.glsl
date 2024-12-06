#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo1 {float u[];};
layout (std430, binding = 1) buffer ssbo2 {float v[];};

// Dimensions of the grids
uniform int width;
uniform int height;

// PDE settings
uniform bool paused;
uniform int boundary_condition;
uniform float dx;
uniform float dt;

// Gray-Scott Reaction Diffusion specific settings
uniform float a;
uniform float b;
uniform float D;

// Color Map Poly 6 Coefficients
uniform vec3 c0;
uniform vec3 c1;
uniform vec3 c2;
uniform vec3 c3;
uniform vec3 c4;
uniform vec3 c5;
uniform vec3 c6;

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
            return u[getPosition(fmod(x, width), fmod(y, height))];
        } else { // Dirichlet Boundary Condition
            return 0.0;
        }
    }

    int position = getPosition(x, y);
    return u[position];
}

// Accesses the value of the V grid at a coordinate while respecting value-based boundary conditions
float V(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 2) { // Periodic Boundary Condition
            return v[getPosition(fmod(x, width), fmod(y, height))];
        } else { // Dirichlet Boundary Condition
            return 0.0;
        }
    }

    int position = getPosition(x, y);
    return v[position];
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

// Use Euler's Method to make one step forward in time
void euler(ivec2 location) {
    float du_dt = du_dt(location.x, location.y);
    float dv_dt = dv_dt(location.x, location.y);

    u[getPosition(location.x, location.y)] = U(location.x, location.y) + du_dt * dt;
    v[getPosition(location.x, location.y)] = V(location.x, location.y) + dv_dt * dt;
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);

    if (!paused) euler(location);

    float luminosity = U(location.x, location.y);
    imageStore(imgOutput, location, vec4(cmap(luminosity * 2.0), 1.0));
}