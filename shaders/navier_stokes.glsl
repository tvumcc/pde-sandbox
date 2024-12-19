#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo1 {float u[];};
layout (std430, binding = 1) buffer ssbo2 {float v[];};
layout (std430, binding = 2) buffer ssbo3 {float p[];};
layout (std430, binding = 3) buffer ssbo4 {float s[];};

// Dimensions of the grids
uniform int width;
uniform int height;

// PDE settings
uniform bool paused;
uniform int boundary_condition;
uniform float dx;
uniform float dt;

// Navier-Stokes Reaction Diffusion specific settings
uniform float viscosity;

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

// Accesses the value of the P grid at a coordinate while respecting value-based boundary conditions
float P(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (boundary_condition == 2) { // Periodic Boundary Condition
            return p[getPosition(fmod(x, width), fmod(y, height))];
        } else { // Dirichlet Boundary Condition
            return 0.0;
        }
    }

    int position = getPosition(x, y);
    return p[position];
}

// Accesses the value of the S grid at a coordinate while respecting value-based boundary conditions
float S(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        if (x < 0) {
            return 1.0;
        } else if (x >= width) {
            return 0.0;
        } else {
            return s[getPosition(fmod(x, width), fmod(y, height))];
        }
    }

    int position = getPosition(x, y);
    return s[position];
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

// Use Euler's Method to make one step forward in time
void euler(ivec2 location) {
    float du_dt = du_dt(location.x, location.y);
    float dv_dt = dv_dt(location.x, location.y);
    float dp_dt = dp_dt(location.x, location.y);
    float ds_dt = ds_dt(location.x, location.y);

    u[getPosition(location.x, location.y)] = U(location.x, location.y) + du_dt * dt;
    v[getPosition(location.x, location.y)] = V(location.x, location.y) + dv_dt * dt;
    p[getPosition(location.x, location.y)] = P(location.x, location.y) + dp_dt * dt;
    s[getPosition(location.x, location.y)] = S(location.x, location.y) + ds_dt * dt;
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);

    if (!paused) euler(location);

    // float luminosity = sqrt(pow(U(location.x, location.y), 2) + pow(V(location.x, location.y), 2));
    float luminosity = S(location.x, location.y);
    imageStore(imgOutput, location, vec4(cmap(abs(min(1.0, luminosity))), 1.0));
}