#pragma once

#include <vulkan/vulkan.hpp>
#include <eseed/window/window.hpp>

namespace esd::gpu {

class RenderContext {
public:
    RenderContext(std::shared_ptr<esd::window::Window> window);
};

}