#include <eseed/graphics/rendering/swapchainmanager.hpp>

using namespace esd::graphics;

SwapchainManager::SwapchainManager(vk::SwapchainKHR swapchain) 
: swapchain(swapchain) {}

vk::SwapchainKHR getSwapchain() {
    return swapchain;
}

void SwapchainManager::setDevice(vk::Device device) {
    
}