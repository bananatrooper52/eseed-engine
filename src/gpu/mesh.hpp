#pragma once

#include <eseed/math/vec.hpp>
#include <vector>

struct Vertex {
    esdm::Vec2<float> position;
    esdm::Vec3<float> color;
};

struct Mesh {
    std::vector<Vertex> vertices;
    Mesh(std::vector<Vertex> vertices) : vertices(vertices) {}
};