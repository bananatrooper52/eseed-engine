#pragma once

#include <eseed/window/window.hpp>
#include <vulkan/vulkan.hpp>
#include <optional>

class Gpu {
public:
    Gpu(std::shared_ptr<esd::window::Window> window = nullptr);
    void render();
    std::vector<uint8_t> loadShaderCode(std::string path);

    vk::Instance getInstance() { return instance; }
    vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
    vk::Device getDevice() { return device; }

private:
    // Used to return queue family indexes along with a device
    struct DeviceInfo {
        vk::Device device;
        std::optional<uint32_t> graphicsQueueFamily;
    };

    // Main objects
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    // Queues
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

    //

    // Create a Vulkan instance
    // If a window is provided, the required extensions will be enabled
    static vk::Instance createInstance(
        bool enableLayers = false,
        std::shared_ptr<esd::window::Window> window = nullptr
    );

    // Find a physical device
    static vk::PhysicalDevice findPhysicalDevice(vk::Instance instance);

    // Create a Vulkan device from the provided instance
    // Returns an object containing the device and the found queue families
    static DeviceInfo createDevice(
        vk::Instance instance,
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface
    );

    // Create a swapchain
    static vk::SwapchainKHR createSwapchain(
        vk::PhysicalDevice physicalDevice,
        vk::Device device,
        vk::SurfaceKHR surface,
        vk::SurfaceFormatKHR surfaceFormat
    );

    static std::vector<vk::ImageView> createSwapchainImageViews(
        vk::Device device, 
        vk::SwapchainKHR swapchain,
        vk::Format format
    );

    static vk::RenderPass createRenderPass(
        vk::Device device,
        vk::SurfaceFormatKHR surfaceFormat
    );

    static vk::Pipeline createRenderPipeline(
        vk::Device device,
        vk::RenderPass renderPass,
        const std::vector<uint8_t>& vertShaderCode,
        const std::vector<uint8_t>& fragShaderCode,
        float width,
        float height
    );

    static vk::ShaderModule createShaderModule(
        vk::Device device,
        const std::vector<uint8_t>& code
    );

    static std::vector<vk::Framebuffer> createSwapchainFramebuffers(
        vk::Device device,
        vk::SwapchainKHR swapchain,
        vk::RenderPass renderPass,
        std::vector<vk::ImageView> imageViews,
        uint32_t width,
        uint32_t height
    );

    static vk::CommandPool createCommandPool(
        vk::Device device,
        uint32_t queueFamily
    );

    static std::vector<vk::CommandBuffer> createCommandBuffers(
        vk::Device device,
        vk::CommandPool commandPool,
        uint32_t count
    );
};