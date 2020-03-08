#pragma once

#include "renderpipeline.hpp"

#include <eseed/window/window.hpp>
#include <vulkan/vulkan.hpp>
#include <optional>
#include <memory>

class RenderContext {
public:
    RenderContext(std::shared_ptr<esd::window::Window> window = nullptr);
    void render();
    std::vector<uint8_t> loadShaderCode(std::string path);

    vk::Instance getInstance() { return instance; }
    vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
    vk::Device getDevice() { return device; }

    vk::Buffer createVertexBuffer(std::vector<esd::math::Vec2<float>> vertices);

private:
    std::unique_ptr<RenderPipeline> renderPipeline;

    // Main objects
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    // Queues
    std::optional<uint32_t> graphicsQueueFamily;
    vk::Queue graphicsQueue;

    // Presentation
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapchain;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    // Commands
    vk::CommandPool commandPool;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;

    // Buffer
    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::MemoryRequirements memoryRequirements;

    void createInstance(
        bool enableLayers = false,
        std::shared_ptr<esd::window::Window> window = nullptr
    );

    void findPhysicalDevice(vk::Instance instance);

    void createDevice(
        vk::Instance instance,
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface
    );

    void createSwapchain(
        vk::PhysicalDevice physicalDevice,
        vk::Device device,
        vk::SurfaceKHR surface,
        vk::SurfaceFormatKHR surfaceFormat
    );

    void createSwapchainImageViews(
        vk::Device device, 
        vk::SwapchainKHR swapchain,
        vk::Format format
    );

    vk::ShaderModule createShaderModule(
        vk::Device device,
        const std::vector<uint8_t>& code
    );

    void createCommandPool(
        vk::Device device,
        uint32_t queueFamily
    );
};