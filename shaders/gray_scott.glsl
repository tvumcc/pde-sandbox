#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo1 {
    float u[];
};
layout (std430, binding = 1) buffer ssbo2 {
    float v[];
};

uniform int width;
uniform int height;
uniform float a;
uniform float b;
uniform float D;

// Credit to https://www.shadertoy.com/view/Nd3fR2 for the MPL color maps

// makes viridis colormap with polynimal 6
vec3 viridis(float t) {
    const vec3 c0 = vec3(0.274344,0.004462,0.331359);
    const vec3 c1 = vec3(0.108915,1.397291,1.388110);
    const vec3 c2 = vec3(-0.319631,0.243490,0.156419);
    const vec3 c3 = vec3(-4.629188,-5.882803,-19.646115);
    const vec3 c4 = vec3(6.181719,14.388598,57.442181);
    const vec3 c5 = vec3(4.876952,-13.955112,-66.125783);
    const vec3 c6 = vec3(-5.513165,4.709245,26.582180);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

// makes Blues_r colormap with polynimal 6
vec3 Blues_r(float t) {
    const vec3 c0 = vec3(0.042660,0.186181,0.409512);
    const vec3 c1 = vec3(-0.703712,1.094974,2.049478);
    const vec3 c2 = vec3(7.995725,-0.686110,-4.998203);
    const vec3 c3 = vec3(-24.421963,2.680736,7.532937);
    const vec3 c4 = vec3(47.519089,-4.615112,-5.126531);
    const vec3 c5 = vec3(-46.038418,2.606781,0.685560);
    const vec3 c6 = vec3(16.586546,-0.279280,0.447047);
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

float V(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0.0; // Dirichlet Boundary Condition
    int position = getPosition(x, y);
    return v[position];
}


float du_dt(ivec2 location, float offset, float dx, float dy) {
    float d2u_dx2 = ((U(location.x+1, location.y) + offset) + (U(location.x-1, location.y) + offset) - 2 * (U(location.x, location.y) + offset)) / (dx * dx);
    float d2u_dy2 = ((U(location.x, location.y+1) + offset) + (U(location.x, location.y-1) + offset) - 2 * (U(location.x, location.y) + offset)) / (dy * dy);
    return (d2u_dx2 + d2u_dy2) + (pow(U(location.x, location.y) + offset, 2) * (V(location.x, location.y))) - (a + b) * (U(location.x, location.y) + offset);
}

float dv_dt(ivec2 location, float offset, float dx, float dy) {
    float d2v_dx2 = ((V(location.x+1, location.y) + offset) + (V(location.x-1, location.y) + offset) - 2 * (V(location.x, location.y) + offset)) / (dx * dx);
    float d2v_dy2 = ((V(location.x, location.y+1) + offset) + (V(location.x, location.y-1) + offset) - 2 * (V(location.x, location.y) + offset)) / (dy * dy);
    return D * (d2v_dx2 + d2v_dy2) - (pow(U(location.x, location.y), 2) * (V(location.x, location.y) + offset)) + a * (1 - (V(location.x, location.y) + offset));
}

void euler(ivec2 location, float dx, float dy, float dt) {
    // float d2u_dx2 = (U(location.x+1, location.y) + U(location.x-1, location.y) - 2 * U(location.x, location.y)) / (dx * dx);
    // float d2u_dy2 = (U(location.x, location.y+1) + U(location.x, location.y-1) - 2 * U(location.x, location.y)) / (dy * dy);
    // float du_dt = (d2u_dx2 + d2u_dy2) + (pow(U(location.x, location.y), 2) * V(location.x, location.y)) - (a + b) * U(location.x, location.y);

    // float d2v_dx2 = (V(location.x+1, location.y) + V(location.x-1, location.y) - 2 * V(location.x, location.y)) / (dx * dx);
    // float d2v_dy2 = (V(location.x, location.y+1) + V(location.x, location.y-1) - 2 * V(location.x, location.y)) / (dy * dy);
    // float dv_dt = D * (d2v_dx2 + d2v_dy2) - (pow(U(location.x, location.y), 2) * V(location.x, location.y)) + a * (1 - V(location.x, location.y));

    float du_dt = du_dt(location, 0.0, dx, dy);
    float dv_dt = dv_dt(location, 0.0, dx, dy);

    u[getPosition(location.x, location.y)] = U(location.x, location.y) + du_dt * dt;
    v[getPosition(location.x, location.y)] = V(location.x, location.y) + dv_dt * dt;
}

void RK4(ivec2 location, float dx, float dy, float dt) {
    float k1u = du_dt(location, 0, dx, dy);
    float k2u = du_dt(location, dt * (k1u / 2.0), dx, dy);
    float k3u = du_dt(location, dt * (k2u / 2.0), dx, dy);
    float k4u = du_dt(location, dt * k3u, dx, dy);
    float du = (dt / 6.0) * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);


    float k1v = dv_dt(location, 0, dx, dy);
    float k2v = dv_dt(location, dt * (k1v / 2.0), dx, dy);
    float k3v = dv_dt(location, dt * (k2v / 2.0), dx, dy);
    float k4v = dv_dt(location, dt * k3v, dx, dy);
    float dv = (dt / 6.0) * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);

    u[getPosition(location.x, location.y)] = U(location.x, location.y) + du;
    v[getPosition(location.x, location.y)] = V(location.x, location.y) + dv;
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);

    float dx = 5.0;
    float dy = 5.0;
    float dt = 1.0;

    euler(location, dx, dy, dt);
    // RK4(location, dx, dy, dt);

    float luminosity = U(location.x, location.y);
    imageStore(imgOutput, location, vec4(viridis(luminosity * 2.0), 1.0));
}