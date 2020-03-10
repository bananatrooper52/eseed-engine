#version 450

layout (set = 0, binding = 0) uniform Camera {
    float x;
} camera; 

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 vertColor;

void main() {
    gl_Position = vec4(position + vec2(camera.x, 0), 0.0, 1.0);
    vertColor = color;
}