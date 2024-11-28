#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo1
{
    float u[];
};

layout (location = 0) uniform int width;
layout (location = 1) uniform int height;

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
    int position = getPosition(x, y);
    if (position < 0 || position >= width * height) return 0.0;
    else return u[position];
}

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);


    float alpha = 3.0;
    float delta_x = 1.0;
    float delta_y = 1.0;
    float dt = 0.1;

    // using dirchlet boundary condition

    float d2u_dx2 = U(location.x+1, location.y) + U(location.x-1, location.y) - 2 * U(location.x, location.y) / (delta_x * delta_x);
    float d2u_dy2 = U(location.x, location.y+1) + U(location.x, location.y-1) - 2 * U(location.x, location.y) / (delta_y * delta_y);
    float du_dt = alpha * (d2u_dx2 + d2u_dy2);


    u[getPosition(location.x, location.y)] = U(location.x, location.y) + du_dt * dt;

    float luminosity = U(location.x, location.y);
    imageStore(imgOutput, location, vec4(viridis(luminosity), 1.0));
}