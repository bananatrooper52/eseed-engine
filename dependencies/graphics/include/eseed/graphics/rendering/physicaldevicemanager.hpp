#pragma once

#include <vulkan/vulkan.hpp>
#include <functional>

namespace esd::graphics {

class PhysicalDeviceManager {
public:
    PhysicalDeviceManager(vk::PhysicalDevice physicalDevice);

    vk::PhysicalDevice getPhysicalDevice() const;

    bool areAllExtensionsAvailable(
        std::vector<const char*> extensionNames
    ) const;

    uint32_t findQueueIndex(
        std::function<bool(uint32_t, vk::QueueFamilyProperties)> fn
    ) const;

    uint32_t findGraphicsQueueIndex() const;

    uint32_t findPresentQueueIndex(vk::SurfaceKHR surface) const;
    
    std::string getName() const;

private:
    vk::PhysicalDevice physicalDevice;
};

}