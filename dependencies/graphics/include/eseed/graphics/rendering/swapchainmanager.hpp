#pragma once

#include <vulkan/vulkan.hpp>

namespace esd::graphics {

class SwapchainManager {
public:
    SwapchainManager(vk::SwapchainKHR swapchain);

    vk::SwapchainKHR getSwapchain() const;

    void setDevice(vk::Device device);

private:
    vk::SwapchainKHR swapchain;
};

}