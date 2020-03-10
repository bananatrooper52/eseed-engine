#pragma once

#include "resourcemanager.hpp"

#include <vulkan/vulkan.hpp>
#include <vector>

class PresentManager {
public: 
    PresentManager(const PresentManager&) = delete;
    PresentManager(
        std::shared_ptr<ResourceManager> rm,
        std::shared_ptr<esd::window::Window> window
    );
    ~PresentManager();

    uint32_t getNextImageIndex(vk::Semaphore semaphore);
    vk::ImageView getImageView(uint32_t index);
    uint32_t getImageCount();

    vk::SwapchainKHR getSwapchain();

    esd::math::Vec2<U32> getSize();

private:
    std::shared_ptr<ResourceManager> rm;

    std::shared_ptr<esd::window::Window> window;

    vk::SurfaceFormatKHR swapchainFormat;
    vk::SwapchainKHR swapchain;
    std::vector<vk::ImageView> swapchainImageViews;

    void findSwapchainFormat();
    void createSwapchain();
    void createSwapchainImageViews();
};