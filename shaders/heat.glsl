#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo0 {
    float u[];
};

uniform bool paused;

uniform int width;
uniform int height;
uniform float alpha;

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

float U(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0.0; // Dirichlet Boundary Condition
    int position = getPosition(x, y);
    return u[position];
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);

    float delta_x = 1.0;
    float delta_y = 1.0;
    float dt = 0.1;

    // using dirchlet boundary condition

    float d2u_dx2 = (U(location.x+1, location.y) + U(location.x-1, location.y) - 2 * U(location.x, location.y)) / (delta_x * delta_x);
    float d2u_dy2 = (U(location.x, location.y+1) + U(location.x, location.y-1) - 2 * U(location.x, location.y)) / (delta_y * delta_y);
    float du_dt = alpha * (d2u_dx2 + d2u_dy2);


    if (!paused) u[getPosition(location.x, location.y)] = U(location.x, location.y) + du_dt * dt;

    float luminosity = U(location.x, location.y);
    imageStore(imgOutput, location, vec4(cmap(luminosity), 1.0));
}