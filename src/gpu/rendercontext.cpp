#include "rendercontext.hpp"

#include <fstream>

RenderContext::RenderContext(std::shared_ptr<esd::window::Window> window) {

    // Create instance
    createInstance(true, window);
    
    // Create surface if window is provided
    surface = window->createSurface(instance);

    // Find physical device
    findPhysicalDevice(instance);

    // Create device
    createDevice(instance, physicalDevice, surface);

    // Extract queues
    graphicsQueue = device.getQueue(*graphicsQueueFamily, 0);

    vk::SurfaceFormatKHR surfaceFormat;
    for (auto format : physicalDevice.getSurfaceFormatsKHR(surface)) {
        surfaceFormat = format;
        break;
    }

    // Create swapchain
    createSwapchain(physicalDevice, device, surface, surfaceFormat);

    createSwapchainImageViews(device, swapchain, surfaceFormat.format);

    auto surfaceCapabilities = 
        physicalDevice.getSurfaceCapabilitiesKHR(surface);

    createCommandPool(device, *graphicsQueueFamily);

    imageAvailableSemaphore = device.createSemaphore({});
    renderFinishedSemaphore = device.createSemaphore({});

    renderPipeline = std::make_unique<RenderPipeline>(
        device,
        swapchainImageViews,
        surfaceFormat,
        commandPool,
        (esd::math::Vec2<float>)window->getSize(),
        createShaderModule(
            device, 
            loadShaderCode("resources/shaders/test/test.vert.spv")
        ),
        createShaderModule(
            device, 
            loadShaderCode("resources/shaders/test/test.frag.spv")
        )
    );

    renderPipeline->registerMeshBuffer(createMeshBuffer(Mesh({
        {{ -0.8, -0.8 }, { 1.f, 1.f, 1.f }},
        {{ -0.6, -0.6 }, { 1.f, 0.f, 1.f }},
        {{ -0.8, -0.6 }, { 0.f, 1.f, 0.f }}
    })));
}

void RenderContext::render() {
    uint32_t imageIndex = device.acquireNextImageKHR(
        swapchain, 
        UINT64_MAX, 
        imageAvailableSemaphore, 
        nullptr
    ).value;

    std::vector<vk::Semaphore> waitSemaphores = {
        imageAvailableSemaphore
    };
    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eTopOfPipe
    };
    std::vector<vk::Semaphore> signalSemaphores = {
        renderFinishedSemaphore
    };
    auto si = vk::SubmitInfo()
        .setWaitSemaphoreCount((uint32_t)waitSemaphores.size())
        .setPWaitSemaphores(waitSemaphores.data())
        .setPWaitDstStageMask(waitStages.data())
        .setSignalSemaphoreCount((uint32_t)signalSemaphores.size())
        .setPSignalSemaphores(signalSemaphores.data())
        .setCommandBufferCount(1)
        .setPCommandBuffers(&renderPipeline->getCommandBuffer(imageIndex));        

    graphicsQueue.submit(1, &si, nullptr);

    auto pi = vk::PresentInfoKHR()
        .setWaitSemaphoreCount((uint32_t)signalSemaphores.size())
        .setPWaitSemaphores(signalSemaphores.data())
        .setSwapchainCount(1)
        .setPSwapchains(&swapchain)
        .setPImageIndices(&imageIndex);

    graphicsQueue.presentKHR(pi);

    graphicsQueue.waitIdle();
}

std::vector<uint8_t> RenderContext::loadShaderCode(std::string path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) throw std::runtime_error("File does not exist");

    std::vector<uint8_t> code;

    file.seekg(0, file.end);
    code.resize(file.tellg());
    file.seekg(0, file.beg);

    file.read((char*)code.data(), code.size());

    return code;
}

std::shared_ptr<MeshBuffer> RenderContext::createMeshBuffer(
    const Mesh& mesh
) {
    std::shared_ptr<MeshBuffer> meshBuffer = std::make_shared<MeshBuffer>();
    meshBuffer->vertexCount = (uint32_t)mesh.vertices.size();
    
    vk::DeviceSize bufferByteLength = 
        sizeof(mesh.vertices[0]) * mesh.vertices.size();

    meshBuffer->buffer = device.createBuffer(vk::BufferCreateInfo()
            .setSize(bufferByteLength)
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive)
    );

    auto memoryRequirements = 
        device.getBufferMemoryRequirements(meshBuffer->buffer);
    auto memoryProperties = physicalDevice.getMemoryProperties();

    uint32_t memoryTypeIndex;
    vk::MemoryPropertyFlags properties = 
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (
            memoryRequirements.memoryTypeBits & (1 << i) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == 
            properties
        ) {
            memoryTypeIndex = i;
            break;
        }
    }

    meshBuffer->memory = device.allocateMemory(vk::MemoryAllocateInfo()
            .setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memoryTypeIndex)
    );

    device.bindBufferMemory(meshBuffer->buffer, meshBuffer->memory, 0);

    void* data;
    device.mapMemory(
        meshBuffer->memory, 
        0, 
        bufferByteLength, 
        vk::MemoryMapFlags(), 
        &data
    );
    memcpy(data, mesh.vertices.data(), (size_t)bufferByteLength);
    device.unmapMemory(meshBuffer->memory);

    return meshBuffer;
}

void RenderContext::createInstance(
    bool enableLayers,
    std::shared_ptr<esd::window::Window> window
) {
    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    // If a window is provided, include its required extensions
    if (window != nullptr) {
        auto windowExtensions = window->getRequiredInstanceExtensionNames();
        extensions.insert(
            extensions.end(), 
            windowExtensions.begin(), 
            windowExtensions.end()
        );
    }

    // If layers are enabled, add them
    if (enableLayers) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    
    instance = vk::createInstance(vk::InstanceCreateInfo()
            .setEnabledExtensionCount((uint32_t)extensions.size())
            .setPpEnabledExtensionNames(extensions.data())
            .setEnabledLayerCount((uint32_t)layers.size())
            .setPpEnabledLayerNames(layers.data())
    );
}

void RenderContext::findPhysicalDevice(vk::Instance instance) {
    // TODO: do a little more thinking than just picking the first option
    physicalDevice = instance.enumeratePhysicalDevices()[0];
}

void RenderContext::createDevice(
    vk::Instance instance, 
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR surface
) {
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
                !physicalDevice.getSurfaceSupportKHR(i, surface)
            ) continue;

            graphicsQueueFamily = i;
        }
    }
    
    // Set up queue create infos
    std::vector<vk::DeviceQueueCreateInfo> queueCis;

    // Graphics
    float graphicsQueuePriorities[] = { 1.f };
    queueCis.push_back(
        vk::DeviceQueueCreateInfo() 
            .setQueueCount(1)
            .setQueueFamilyIndex(*graphicsQueueFamily)
            .setPQueuePriorities(graphicsQueuePriorities)
    );
    
    std::vector<const char*> extensions;

    // If a surface is provided, we are going to want to include swapchain
    // extension
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    device = physicalDevice.createDevice(vk::DeviceCreateInfo()
            .setQueueCreateInfoCount((uint32_t)queueCis.size())
            .setPQueueCreateInfos(queueCis.data())
            .setEnabledExtensionCount((uint32_t)extensions.size())
            .setPpEnabledExtensionNames(extensions.data())
    );
}

void RenderContext::createSwapchain(
    vk::PhysicalDevice physicalDevice,
    vk::Device device,
    vk::SurfaceKHR surface,
    vk::SurfaceFormatKHR surfaceFormat
) {
    auto surfaceCapabilities = 
        physicalDevice.getSurfaceCapabilitiesKHR(surface);

    swapchain = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
            .setMinImageCount(surfaceCapabilities.minImageCount)
            .setImageExtent(surfaceCapabilities.maxImageExtent)
            .setImageFormat(surfaceFormat.format)
            .setPreTransform(
                surfaceCapabilities.currentTransform
            )
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageArrayLayers(1)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setClipped(true)
    );
}

void RenderContext::createSwapchainImageViews(
    vk::Device device,
    vk::SwapchainKHR swapchain,
    vk::Format format
) {
    auto images = device.getSwapchainImagesKHR(swapchain);

    swapchainImageViews.resize(images.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        auto imageViewCi = vk::ImageViewCreateInfo()
            .setImage(images[i])
            .setFormat(format)
            .setComponents(vk::ComponentMapping {}) // Empty cnstr, all identity
            .setViewType(vk::ImageViewType::e2D)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
            );
        swapchainImageViews[i] = device.createImageView(imageViewCi);
    }
}

vk::ShaderModule RenderContext::createShaderModule(
    vk::Device device,
    const std::vector<uint8_t>& code
) {
    auto shaderModuleCi = vk::ShaderModuleCreateInfo()
        .setCodeSize(code.size())
        .setPCode((uint32_t*)code.data());

    return device.createShaderModule(shaderModuleCi);
}

void RenderContext::createCommandPool(
    vk::Device device,
    uint32_t queueFamily
) {
    commandPool = device.createCommandPool(vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(queueFamily)
    );
}
