#pragma once

#include "resourcemanager.hpp"

#include <vulkan/vulkan.hpp>
#include <vector>

class PresentManager {
public: 
    PresentManager(const PresentManager&) = delete;
    PresentManager(std::shared_ptr<ResourceManager> rm);

    uint32_t getNextImageIndex(vk::Semaphore semaphore);
    vk::ImageView getImageView(uint32_t index);

    vk::SwapchainKHR getSwapchain();

private:
    std::shared_ptr<ResourceManager> rm;

    vk::SurfaceFormatKHR swapchainFormat;
    vk::SwapchainKHR swapchain;
    std::vector<vk::ImageView> swapchainImageViews;

    void findSwapchainFormat();
    void createSwapchain();
    void createSwapchainImageViews();
};