#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo1 {
    float u[];
};
layout (std430, binding = 1) buffer ssbo2 {
    float v[];
};

uniform bool paused;
uniform int boundary_condition;

uniform int width;
uniform int height;
uniform float a;
uniform float b;
uniform float D;

uniform float dx;
uniform float dt;

// Color Map Poly 6 Coefficients
uniform vec3 c0;
uniform vec3 c1;
uniform vec3 c2;
uniform vec3 c3;
uniform vec3 c4;
uniform vec3 c5;
uniform vec3 c6;

vec3 cmap(float t) {
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

int getPosition(int x, int y) {
    return (y * width) + x;
}

int fmod(int x, int y) {
    return (x % y + y) % y;
}

float U(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 0) { // Dirichlet Boundary Condition
            return 0.0;
        } else if (boundary_condition == 2) { // Periodic Boundary Condition
            return u[getPosition(fmod(x, width), fmod(y, height))];
        }
    }

    int position = getPosition(x, y);
    return u[position];
}

float V(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 0) { // Dirichlet Boundary Condition
            return 0.0;
        } else if (boundary_condition == 2) { // Periodic Boundary Condition
            return v[getPosition(fmod(x, width), fmod(y, height))];
        }
    }

    int position = getPosition(x, y);
    return v[position];
}


float du_dt(int x, int y, float offset) {
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

    return (d2u_dx2 + d2u_dy2) + (pow(U(x, y) + offset, 2) * (V(x, y))) - (a + b) * (U(x, y) + offset);
}

float dv_dt(int x, int y, float offset) {
    float dv_dx_0 = (V(x, y) - V(x-1, y)) / dx;
    float dv_dx_1 = (V(x+1, y) - V(x, y)) / dx;

    float dv_dy_0 = (V(x, y) - V(x, y-1)) / dx;
    float dv_dy_1 = (V(x, y+1) - V(x, y)) / dx;

    if (boundary_condition == 1) {
        if (x == 0) dv_dx_0 = 0.0;
        if (x == width-1) dv_dx_1 = 0.0;
        if (y == 0) dv_dy_0 = 0.0;
        if (y == height-1) dv_dy_1 = 0.0;
    }

    float d2v_dx2 = (dv_dx_1 - dv_dx_0) / dx;
    float d2v_dy2 = (dv_dy_1 - dv_dy_0) / dx;

    return D * (d2v_dx2 + d2v_dy2) - (pow(U(x, y), 2) * (V(x, y) + offset)) + a * (1 - (V(x, y) + offset));
}

void euler(ivec2 location, float dx, float dy, float dt) {
    // float d2u_dx2 = (U(location.x+1, location.y) + U(location.x-1, location.y) - 2 * U(location.x, location.y)) / (dx * dx);
    // float d2u_dy2 = (U(location.x, location.y+1) + U(location.x, location.y-1) - 2 * U(location.x, location.y)) / (dy * dy);
    // float du_dt = (d2u_dx2 + d2u_dy2) + (pow(U(location.x, location.y), 2) * V(location.x, location.y)) - (a + b) * U(location.x, location.y);

    // float d2v_dx2 = (V(location.x+1, location.y) + V(location.x-1, location.y) - 2 * V(location.x, location.y)) / (dx * dx);
    // float d2v_dy2 = (V(location.x, location.y+1) + V(location.x, location.y-1) - 2 * V(location.x, location.y)) / (dy * dy);
    // float dv_dt = D * (d2v_dx2 + d2v_dy2) - (pow(U(location.x, location.y), 2) * V(location.x, location.y)) + a * (1 - V(location.x, location.y));

    float du_dt = du_dt(location.x, location.y, 0.0);
    float dv_dt = dv_dt(location.x, location.y, 0.0);

    u[getPosition(location.x, location.y)] = U(location.x, location.y) + du_dt * dt;
    v[getPosition(location.x, location.y)] = V(location.x, location.y) + dv_dt * dt;
}

// void RK4(ivec2 location, float dx, float dy, float dt) {
//     float k1u = du_dt(location, 0, dx, dy);
//     float k2u = du_dt(location, dt * (k1u / 2.0), dx, dy);
//     float k3u = du_dt(location, dt * (k2u / 2.0), dx, dy);
//     float k4u = du_dt(location, dt * k3u, dx, dy);
//     float du = (dt / 6.0) * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);


//     float k1v = dv_dt(location, 0, dx, dy);
//     float k2v = dv_dt(location, dt * (k1v / 2.0), dx, dy);
//     float k3v = dv_dt(location, dt * (k2v / 2.0), dx, dy);
//     float k4v = dv_dt(location, dt * k3v, dx, dy);
//     float dv = (dt / 6.0) * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);

//     u[getPosition(location.x, location.y)] = U(location.x, location.y) + du;
//     v[getPosition(location.x, location.y)] = V(location.x, location.y) + dv;
// }

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);

    if (!paused) euler(location, dx, dx, dt);
    // RK4(location, dx, dy, dt);

    float luminosity = U(location.x, location.y);
    imageStore(imgOutput, location, vec4(cmap(luminosity * 2.0), 1.0));
}