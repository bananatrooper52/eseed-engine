#pragma once

#include <eseed/math/vec.hpp>
#include <vector>

struct Vertex {
    esd::math::Vec2<float> position;
    esd::math::Vec3<float> color;
};

struct Mesh {
    std::vector<Vertex> vertices;
    Mesh(std::vector<Vertex> vertices) : vertices(vertices) {}
};