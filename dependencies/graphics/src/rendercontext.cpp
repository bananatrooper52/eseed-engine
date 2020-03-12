#include <eseed/graphics/rendering/rendercontext.hpp>

using namespace esd::graphics;
using namespace esdl;
using namespace esdm;

RenderContext::RenderContext(std::shared_ptr<Window> window, bool debugEnabled) 
: window(window), debugEnabled(debugEnabled) {}

void RenderContext::init() {
    initInstanceManager();
    initSurfaceManager();
    initPhysicalDeviceManager();
    initDeviceManager();
    initQueues();
    initSwapchainManager();
}

void RenderContext::render() {

}

void RenderContext::initInstanceManager() {
    auto extensionNames = getRequiredInstanceExtensionNames();
    auto layerNames = getRequiredInstanceLayerNames();
    
    auto instanceCi = vk::InstanceCreateInfo()
        .setEnabledExtensionCount((uint32_t)extensionNames.size())
        .setPpEnabledExtensionNames(extensionNames.data())
        .setEnabledLayerCount((uint32_t)layerNames.size())
        .setPpEnabledLayerNames(layerNames.data());
    
    instanceManager = std::make_unique<InstanceManager>(
        vk::createInstance(instanceCi)
    );

    mainLogger.debug("Initialized render context instance manager");
}

void RenderContext::initSurfaceManager() {
    surfaceManager = std::make_unique<SurfaceManager>(
        window->createSurface(instanceManager->getInstance())
    );

    mainLogger.debug("Initialized render context surface manager");
}

void RenderContext::initPhysicalDeviceManager() {
    mainLogger.fatalAssert(
        instanceManager != nullptr, 
        "Instance manager must be initialized before physical device manager!"
    );
    
    physicalDeviceManager = std::make_unique<PhysicalDeviceManager>(
        instanceManager->getBestPhysicalDevice(
            getRequiredDeviceExtensionNames()
        )
    );

    mainLogger.debug("Initialized render context physical device manager");
}

void RenderContext::initDeviceManager() {
    mainLogger.fatalAssert(
        physicalDeviceManager != nullptr, 
        "Physical device manager must be initialized before logical device manager!"
    );

    mainLogger.fatalAssert(
        surfaceManager != nullptr,
        "Surface manager must be initialized before logical device manager!"
    );

    std::vector<vk::DeviceQueueCreateInfo> queueCiList;

    uint32_t graphicsQueueIndex = 
        physicalDeviceManager->findGraphicsQueueIndex();
    
    uint32_t presentQueueIndex = 
        physicalDeviceManager->findPresentQueueIndex(
            surfaceManager->getSurface()
        );
    
    // Add graphics queue
    float graphicsQueuePriorities[] = { 1.f };
    queueCiList.push_back(vk::DeviceQueueCreateInfo()
        .setQueueCount(1)
        .setQueueFamilyIndex(graphicsQueueIndex)
        .setPQueuePriorities(graphicsQueuePriorities)
    );

    // Add separate present queue if different from graphics queue
    if (graphicsQueueIndex != presentQueueIndex) {
        float presentQueuePriorities[] = { 1.f };
        queueCiList.push_back(vk::DeviceQueueCreateInfo()
            .setQueueCount(1)
            .setQueueFamilyIndex(presentQueueIndex)
            .setPQueuePriorities(presentQueuePriorities)
        );
    }

    auto extensionNames = getRequiredDeviceExtensionNames();

    auto ci = vk::DeviceCreateInfo()
        .setEnabledExtensionCount((uint32_t)extensionNames.size())
        .setPpEnabledExtensionNames(extensionNames.data())
        .setEnabledLayerCount(0)
        .setQueueCreateInfoCount((uint32_t)queueCiList.size())
        .setPQueueCreateInfos(queueCiList.data());

    deviceManager = std::make_unique<DeviceManager>(
        physicalDeviceManager->getPhysicalDevice().createDevice(ci)
    );

    mainLogger.debug("Initialized render context logical device manager");
}

void RenderContext::initQueues() {
    mainLogger.fatalAssert(
        deviceManager != nullptr,
        "Logical device manager must be initialized before command queues!"
    );

    graphicsQueue = deviceManager->getDevice().getQueue(
        physicalDeviceManager->findGraphicsQueueIndex(),
        0
    );

    presentQueue = deviceManager->getDevice().getQueue(
        physicalDeviceManager->findPresentQueueIndex(
            surfaceManager->getSurface()
        ),
        0
    );
}

void RenderContext::initSwapchainManager() {
    mainLogger.fatalAssert(
        deviceManager != nullptr,
        "Logical device manager must be initialized before swapchain manager!"
    );

    mainLogger.fatalAssert(
        surfaceManager != nullptr,
        "Surface manager must be initialized before swapchain manager!"
    );

    auto physicalDevice = physicalDeviceManager->getPhysicalDevice();
    auto surface = surfaceManager->getSurface();

    auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
    auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(
        surfaceManager->getSurface()
    );

    // Find most suitable surface format
    vk::SurfaceFormatKHR surfaceFormat;
    for (auto surfaceFormatOption : surfaceFormats) {
        bool formatAcceptable = 
            surfaceFormatOption.format == vk::Format::eB8G8R8A8Unorm;
        bool colorSpaceAcceptable =
            surfaceFormatOption.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            
        if (formatAcceptable && colorSpaceAcceptable) {
            surfaceFormat = surfaceFormatOption;
            break;
        }
    }

    // Find most suitable present mode
    vk::PresentModeKHR surfacePresentMode;
    for (auto surfacePresentModeOption : surfacePresentModes) {
        if (surfacePresentModeOption == vk::PresentModeKHR::eMailbox) {
            surfacePresentMode = surfacePresentModeOption;
            break;
        }
    }

    auto ci = vk::SwapchainCreateInfoKHR()
        .setSurface(surfaceManager->getSurface())
        .setMinImageCount(surfaceCapabilities.minImageCount + 1)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent({ window->getSize().x, window->getSize().y })
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setQueueFamilyIndexCount(0)
        .setPreTransform(surfaceCapabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(surfacePresentMode)
        .setClipped(true);

    swapchainManager = std::make_unique<SwapchainManager>(
        deviceManager->getDevice().createSwapchainKHR(ci)
    );

    mainLogger.debug("Initialized render context swapchain manager");
}

std::vector<const char*> RenderContext::getRequiredInstanceExtensionNames() {
    std::vector<const char*> windowExtensionNames = 
        window->getRequiredInstanceExtensionNames();

    std::vector<const char*> extensionNames = {};
    extensionNames.insert(
        extensionNames.end(), 
        windowExtensionNames.begin(), 
        windowExtensionNames.end()
    );

    return extensionNames;
}

std::vector<const char*> RenderContext::getRequiredInstanceLayerNames() {
    std::vector<const char*> layerNames = {
        "VK_LAYER_KHRONOS_validation"
    };

    return layerNames;
}

std::vector<const char*> RenderContext::getRequiredDeviceExtensionNames() {
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}