#pragma once

#include <vulkan/vulkan.hpp>

namespace esd::graphics {

class SurfaceManager {
public:
    SurfaceManager(vk::SurfaceKHR surface);

    vk::SurfaceKHR getSurface() const;

private:
    vk::SurfaceKHR surface;
};

}