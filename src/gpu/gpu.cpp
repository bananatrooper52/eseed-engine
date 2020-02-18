#include "gpu.hpp"

#include <eseed/logging/logger.hpp>

using namespace esd;
using namespace esd::window;
using namespace esd::logging;

Gpu::Gpu(std::shared_ptr<window::Window> window) {
    initInstance();
    initSurface();
    choosePhysicalDevice();
    initDevice();
}

vk::Instance Gpu::getInstance() {
    return instance;
}

vk::SurfaceKHR Gpu::getSurface() {
    return surface;
}

vk::PhysicalDevice Gpu::getPhysicalDevice() {
    return physicalDevice;
}

vk::Device Gpu::getDevice() {
    return device;
}

void Gpu::initInstance() {
    auto requiredInstanceExtensionNames = getRequiredInstanceExtensionNames();
    auto requiredInstanceLayerNames = getRequiredInstanceLayerNames();
    
    // TODO: Application info
    auto ci = vk::InstanceCreateInfo()
        .setEnabledExtensionCount((uint32_t)requiredInstanceExtensionNames.size())
        .setPpEnabledExtensionNames(requiredInstanceExtensionNames.data())
        .setEnabledLayerCount((uint32_t)requiredInstanceLayerNames.size())
        .setPpEnabledLayerNames(requiredInstanceLayerNames.data());

    instance = vk::createInstance(ci);
}

void Gpu::choosePhysicalDevice() {
    // TODO: get best physical device rather than first, possibly by conditions
    physicalDevice = instance.enumeratePhysicalDevices()[0];
}

void Gpu::initDevice() {
    initQueueFamilyIndices();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float graphicsPriorities[] = { 1 };
    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
        .setPQueuePriorities(graphicsPriorities)
        .setQueueCount(1)
        .setQueueFamilyIndex(graphicsQueueFamilyIndex));
    
    auto requiredDeviceExtensionNames = getRequiredDeviceExtensionNames();
    
    auto ci = vk::DeviceCreateInfo()
        .setEnabledExtensionCount((uint32_t)requiredDeviceExtensionNames.size())
        .setPpEnabledExtensionNames(requiredDeviceExtensionNames.data())
        .setQueueCreateInfoCount((uint32_t)queueCreateInfos.size())
        .setPQueueCreateInfos(queueCreateInfos.data());
}

std::vector<const char*> Gpu::getRequiredInstanceExtensionNames() {
    std::vector<const char*> requiredExtensionNames = {};
    
    // If a window is specified, include window-specific required extensions
    if (window != nullptr) {
        std::vector<const char*> requiredWindowExtensionNames =
            window->getRequiredInstanceExtensionNames();

        requiredExtensionNames.insert(
            requiredExtensionNames.end(),
            requiredWindowExtensionNames.begin(),
            requiredWindowExtensionNames.end()
        );
    }

    return requiredExtensionNames;
}

std::vector<const char*> Gpu::getRequiredInstanceLayerNames() {
    // TODO: disable validation layers in release
    std::vector<const char*> requiredLayerNames = {
        "VK_LAYER_KHRONOS_validation"
    };

    return requiredLayerNames;
}

std::vector<const char*> Gpu::getRequiredDeviceExtensionNames() {
    std::vector<const char*> requiredExtensionNames;

    // If a window is specified, make swapchain available
    if (window != nullptr)
        requiredExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    return requiredExtensionNames;
}

void Gpu::initQueueFamilyIndices() {
    initGraphicsQueueFamilyIndex();
}

void Gpu::initGraphicsQueueFamilyIndex() {
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
        bool hasGraphicsFlag = bool(queueFamilyProperties[i].queueFlags & 
            vk::QueueFlagBits::eGraphics);

        bool hasSurfaceSupport = 
            physicalDevice.getSurfaceSupportKHR(i, surface);

        if (hasGraphicsFlag && hasSurfaceSupport) {
            graphicsQueueFamilyIndex = i;
            return;
        }
    }
}