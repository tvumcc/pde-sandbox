#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (std430, binding = 0) buffer ssbo1
{
    float u[];
};

layout (location = 0) uniform int width;
layout (location = 1) uniform int height;

void main() {
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);

    int position = (location.y * width) + location.x;
    float luminosity = u[position];

    imageStore(imgOutput, location, luminosity * vec4(1.0, 1.0, 1.0, 1.0));
}