#pragma once

#include <vulkan/vulkan.hpp>

struct MeshBuffer {
    vk::DeviceMemory memory;
    vk::Buffer buffer;
    uint32_t vertexCount;
};