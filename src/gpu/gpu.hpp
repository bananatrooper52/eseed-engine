#pragma once

#include <vulkan/vulkan.hpp>
#include <eseed/window/window.hpp>

namespace esd {

class Gpu {
public:
    Gpu(std::shared_ptr<window::Window> window = nullptr);
    vk::Instance getInstance();
    vk::SurfaceKHR getSurface();
    vk::PhysicalDevice getPhysicalDevice();
    vk::Device getDevice();

private:
    std::shared_ptr<window::Window> window;
    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    uint32_t graphicsQueueFamilyIndex;
    vk::Queue graphicsQueue, presentQueue;

    void initInstance();
    void initSurface();
    void choosePhysicalDevice();
    void initDevice();

    std::vector<const char*> getRequiredInstanceExtensionNames();
    std::vector<const char*> getRequiredInstanceLayerNames();

    std::vector<const char*> getRequiredDeviceExtensionNames();

    void initQueueFamilyIndices();
    void initGraphicsQueueFamilyIndex();
};

}