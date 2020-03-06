#pragma once

#include <eseed/window/window.hpp>
#include <vulkan/vulkan.hpp>
#include <optional>

class RenderContext {
public:
    RenderContext(std::shared_ptr<esd::window::Window> window = nullptr);
    void render();
    std::vector<uint8_t> loadShaderCode(std::string path);

    vk::Instance getInstance() { return instance; }
    vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
    vk::Device getDevice() { return device; }

private:

    // Main objects
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    // Queues
    std::optional<uint32_t> graphicsQueueFamily;
    vk::Queue graphicsQueue;

    // Graphics
    vk::RenderPass renderPass;
    vk::Pipeline pipeline;

    // Presentation
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapchain;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    // Commands
    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;
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

    void createRenderPass(
        vk::Device device,
        vk::SurfaceFormatKHR surfaceFormat
    );

    void createRenderPipeline(
        vk::Device device,
        vk::RenderPass renderPass,
        const std::vector<uint8_t>& vertShaderCode,
        const std::vector<uint8_t>& fragShaderCode,
        float width,
        float height
    );

    vk::ShaderModule createShaderModule(
        vk::Device device,
        const std::vector<uint8_t>& code
    );

    void createSwapchainFramebuffers(
        vk::Device device,
        vk::SwapchainKHR swapchain,
        vk::RenderPass renderPass,
        std::vector<vk::ImageView> imageViews,
        uint32_t width,
        uint32_t height
    );

    void createCommandPool(
        vk::Device device,
        uint32_t queueFamily
    );

    void createCommandBuffers(
        vk::Device device,
        vk::CommandPool commandPool,
        uint32_t count
    );

    void recordCommandBuffers();
};