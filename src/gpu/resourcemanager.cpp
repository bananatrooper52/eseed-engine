#include "resourcemanager.hpp"

ResourceManager::ResourceManager(
    const std::vector<const char*>& instanceExtensionNames,
    const std::vector<const char*>& instanceLayerNames,
    const std::vector<const char*>& deviceExtensionNames,
    std::shared_ptr<esd::window::Window> window
) {

    // -- INSTANCE -- //
    
    instance = vk::createInstance(vk::InstanceCreateInfo()
        .setEnabledExtensionCount((uint32_t)instanceExtensionNames.size())
        .setPpEnabledExtensionNames(instanceExtensionNames.data())
        .setEnabledLayerCount((uint32_t)instanceLayerNames.size())
        .setPpEnabledLayerNames(instanceLayerNames.data())
    );

    // -- SURFACE (IF WINDOW IS PRESENT) -- //

    if (window) surface = window->createSurface(instance);

    // -- PHYSICAL DEVICE -- //

    physicalDevice = instance.enumeratePhysicalDevices()[0];

    // -- DEVICE -- //

    // Locate queue families
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
        
        // Find suitable graphics queue
        if (
            !graphicsQueueFamily &&
            queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics
        ) {
            // If a surface was provided, make sure present queue is supported
            if (
                surface &&
                !physicalDevice.getSurfaceSupportKHR(i, *surface)
            ) continue;

            graphicsQueueFamily = i;
        }
    }
    
    // Set up queue create infos
    std::vector<vk::DeviceQueueCreateInfo> queueCis;

    // Graphics
    float graphicsQueuePriorities[] = { 1.f };
    queueCis.push_back(vk::DeviceQueueCreateInfo() 
        .setQueueCount(1)
        .setQueueFamilyIndex(*graphicsQueueFamily)
        .setPQueuePriorities(graphicsQueuePriorities)
    );

    device = physicalDevice.createDevice(vk::DeviceCreateInfo()
        .setQueueCreateInfoCount((uint32_t)queueCis.size())
        .setPQueueCreateInfos(queueCis.data())
        .setEnabledExtensionCount((uint32_t)deviceExtensionNames.size())
        .setPpEnabledExtensionNames(deviceExtensionNames.data())
    );
}

std::optional<vk::SurfaceCapabilitiesKHR> 
    ResourceManager::getSurfaceCapabilities()
{
    if (!surface) return std::nullopt;
    return physicalDevice.getSurfaceCapabilitiesKHR(*surface);
}

std::optional<std::vector<vk::SurfaceFormatKHR>>
    ResourceManager::getSurfaceFormats()
{
    if (!surface) return std::nullopt;
    return physicalDevice.getSurfaceFormatsKHR(*surface);
}