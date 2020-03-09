#include "rendercontext.hpp"

#include <fstream>

RenderContext::RenderContext(std::shared_ptr<esd::window::Window> window) {

    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;

    // If a window is provided, include its required extensions
    if (window != nullptr) {
        auto windowExtensions = window->getRequiredInstanceExtensionNames();
        instanceExtensions.insert(
            instanceExtensions.end(), 
            windowExtensions.begin(), 
            windowExtensions.end()
        );
    }

    // If layers are enabled, add them
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");

    std::vector<const char*> deviceExtensions;
    if (window) {
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    resourceManager = std::make_shared<ResourceManager>(
        instanceExtensions,
        instanceLayers,
        deviceExtensions,
        window ? window->createSurface() : std::nullopt
    );

    createCommandPool(device, *graphicsQueueFamily);

    imageAvailableSemaphore = device.createSemaphore({});
    renderFinishedSemaphore = device.createSemaphore({});

    renderPipeline = std::make_shared<RenderPipeline>(
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
        {{ -1, -1 }, { 1, 0, 0 }},
        {{ 1, -1 }, { 0, 1, 0 }},
        {{ -1, 1 }, { 0, 0, 1 }},
        {{ -1, 1 }, { 0, 0, 1 }},
        {{ 1, -1 }, { 0, 1, 0 }},
        {{ 1, 1 }, { 1, 0, 1 }},
    })));
}

void RenderContext::render() {
    uint32_t imageIndex = presentManager->getNextImageIndex();

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
        .setPSwapchains(&presentManager->getSwapchain())
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
