#pragma once

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <eseed/math/types.hpp>
#include <eseed/math/vec.hpp>
#include <eseed/logging/logger.hpp>

namespace esd::graphics
{

class Window {
public:
    // Poll for window events
    virtual void poll() = 0;

    // Check if the close button has been pressed and close is requested
    virtual bool isCloseRequested() = 0;

    // Display newly drawn graphics
    virtual void update() = 0;

    // Get a list of extension names required to create a Vulkan Surface
    virtual std::vector<const char*> getRequiredInstanceExtensionNames() = 0;

    // Create a Vulkan surface
    virtual vk::SurfaceKHR createSurface(vk::Instance instance) = 0;

    // Get dimensions of the window in pixels
    virtual esdm::Vec2<I32> getSize() = 0;
};

// Platform-agnostic function to create a window for the native platform
std::unique_ptr<Window> createWindow(
    esdm::Vec2<I32> size, 
    std::string title
);

}