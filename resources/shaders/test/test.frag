#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform Camera {
    vec3 position;
    mat4 rotation;
    float aspect;
} camera; 

layout (location = 0) in vec3 vertColor;
layout (location = 1) in vec2 vertPosition;

layout (location = 0) out vec4 fragColor;

struct Sphere {
    vec3 c;
    float r;
    vec4 color;
};

struct Ray {
    vec3 o;
    vec3 d;
};

struct RayHit {
    float dist;
    vec4 color;
};

Sphere sphere = Sphere(
    vec3(0, 0, -4),
    1,
    vec4(1, 0, 1, 1)
);

bool raySphere(Ray ray, Sphere sphere, out RayHit rayHit) {
    vec3 diff = sphere.c - ray.o;
    float tca = dot(diff, ray.d);
    float distSq = dot(diff, diff) - tca * tca;

    float rSq = sphere.r * sphere.r;
    
    if (distSq > rSq) return false;

    float thc = sqrt(rSq - distSq);

    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 > t1) rayHit.dist = t0;
    else rayHit.dist = t1;

    rayHit.color = sphere.color;
    rayHit.color.rgb = clamp(rayHit.color.rgb * 0.5 * abs(t0 - t1), vec3(0), vec3(1));

    return true;
}

void main() {

    Ray ray = Ray(camera.position, normalize(vec4(vertPosition / vec2(1, camera.aspect), -1, 0) * camera.rotation).xyz);
    
    RayHit rayHit;
    if (raySphere(ray, sphere, rayHit)) {
        fragColor = vec4(min(vec3(1, 1, 1), rayHit.color.rgb), rayHit.color.a);
    } else {
        fragColor = vec4(0, 0, 0, 1);
    }
}