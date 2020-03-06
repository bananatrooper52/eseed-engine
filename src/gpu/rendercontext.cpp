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

    createRenderPass(device, surfaceFormat);

    createRenderPipeline(
        device,
        renderPass,
        loadShaderCode("./resources/shaders/test/test.vert.spv"),
        loadShaderCode("./resources/shaders/test/test.frag.spv"),
        (float)surfaceCapabilities.maxImageExtent.width,
        (float)surfaceCapabilities.maxImageExtent.height
    );

    createSwapchainFramebuffers(
        device, 
        swapchain, 
        renderPass, 
        swapchainImageViews,
        surfaceCapabilities.maxImageExtent.width,
        surfaceCapabilities.maxImageExtent.height
    );

    createCommandPool(device, *graphicsQueueFamily);

    createCommandBuffers(
        device, 
        commandPool, 
        (uint32_t)swapchainImageViews.size()
    );

    imageAvailableSemaphore = device.createSemaphore({});
    renderFinishedSemaphore = device.createSemaphore({});

    std::vector<esd::math::Vec2<float>> vertices = {
        { 0, -0.5 },
        { 0.5, 0.5 },
        { -0.5, 0.5 }
    };

    vk::DeviceSize vertexBufferByteLength = 
        sizeof(vertices[0]) * vertices.size();

    vertexBuffer = device.createBuffer(vk::BufferCreateInfo()
            .setSize(vertexBufferByteLength)
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive)
    );
    memoryRequirements = device.getBufferMemoryRequirements(vertexBuffer);
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

    vertexBufferMemory = device.allocateMemory(vk::MemoryAllocateInfo()
            .setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memoryTypeIndex)
    );

    device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);

    void* data;
    device.mapMemory(
        vertexBufferMemory, 
        0, 
        vertexBufferByteLength, 
        vk::MemoryMapFlags(), 
        &data
    );
    memcpy(data, vertices.data(), (size_t)vertexBufferByteLength);
    device.unmapMemory(vertexBufferMemory);

    recordCommandBuffers();
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
        .setPCommandBuffers(&commandBuffers[imageIndex]);        

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

void RenderContext::createRenderPass(
    vk::Device device,
    vk::SurfaceFormatKHR surfaceFormat
) {
    // Attachment should be cleared at start of render and saved at end
    // We don't care about initial layout of attachment since it will be blank
    // However, we will be displaying it directly to surface, so optimize
    // for that
    auto colorAttachmentDesc = vk::AttachmentDescription()
        .setFormat(surfaceFormat.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    // The color attachment is bound to location 0 in shader
    auto colorAttachmentRef = vk::AttachmentReference()
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto subpassDesc = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentRef);

    renderPass = device.createRenderPass(vk::RenderPassCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&colorAttachmentDesc)
            .setSubpassCount(1)
            .setPSubpasses(&subpassDesc)
    );
}

void RenderContext::createRenderPipeline(
    vk::Device device,
    vk::RenderPass renderPass,
    const std::vector<uint8_t>& vertShaderCode,
    const std::vector<uint8_t>& fragShaderCode,
    float width,
    float height
) {
    // Create shaders
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCis;
    shaderStageCis.push_back(vk::PipelineShaderStageCreateInfo() // Vertex
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(createShaderModule(device, vertShaderCode))
        .setPName("main")
    );
    shaderStageCis.push_back(vk::PipelineShaderStageCreateInfo() // Fragment
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(createShaderModule(device, fragShaderCode))
        .setPName("main")
    );

    auto vertexBindingDescription = vk::VertexInputBindingDescription()
        .setBinding(0)
        .setStride(sizeof(float) * 2)
        .setInputRate(vk::VertexInputRate::eVertex);

    std::array<vk::VertexInputAttributeDescription, 1> 
        vertexAttributeDescriptions;

    vertexAttributeDescriptions[0]
        .setBinding(0)
        .setLocation(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(0);

    auto vertexInputStateCi = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount(1)
        .setPVertexBindingDescriptions(&vertexBindingDescription)
        .setVertexAttributeDescriptionCount(1)
        .setPVertexAttributeDescriptions(vertexAttributeDescriptions.data());

    auto inputAssemblyStateCi = vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList);

    // Viewport covers entire width and height
    auto viewport = vk::Viewport()
        .setMinDepth(0)
        .setMaxDepth(1)
        .setWidth(width)
        .setHeight(height);

    // Scissor covers entire width and height
    auto scissor = vk::Rect2D()
        .setExtent(vk::Extent2D((uint32_t)width, (uint32_t)height));

    auto viewportStateCi = vk::PipelineViewportStateCreateInfo()
        .setViewportCount(1)
        .setPViewports(&viewport)
        .setScissorCount(1)
        .setPScissors(&scissor);
    
    auto rasterizationStateCi = vk::PipelineRasterizationStateCreateInfo()
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise);

    // No multisampling
    auto multisampleStateCi = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    // No color blending
    auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
        .setColorWriteMask(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        );

    auto colorBlendStateCi = vk::PipelineColorBlendStateCreateInfo()
        .setAttachmentCount(1)
        .setPAttachments(&colorBlendAttachment);

    // TODO: No uniforms
    auto pipelineLayoutCi = vk::PipelineLayoutCreateInfo();

    auto pipelineLayout = device.createPipelineLayout(pipelineLayoutCi);

    pipeline = device.createGraphicsPipeline(
        nullptr, 
        vk::GraphicsPipelineCreateInfo()
            .setStageCount(2)
            .setPStages(shaderStageCis.data())
            .setPVertexInputState(&vertexInputStateCi)
            .setPInputAssemblyState(&inputAssemblyStateCi)
            .setPViewportState(&viewportStateCi)
            .setPRasterizationState(&rasterizationStateCi)
            .setPMultisampleState(&multisampleStateCi)
            .setPColorBlendState(&colorBlendStateCi)
            .setLayout(pipelineLayout)
            .setRenderPass(renderPass)
            .setSubpass(0)
            .setLayout(pipelineLayout)
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

void RenderContext::createSwapchainFramebuffers(
    vk::Device device,
    vk::SwapchainKHR swapchain,
    vk::RenderPass renderPass,
    std::vector<vk::ImageView> imageViews,
    uint32_t width,
    uint32_t height
) {
    swapchainFramebuffers.resize(imageViews.size());

    for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
        auto framebufferCi = vk::FramebufferCreateInfo()
            .setRenderPass(renderPass)
            .setAttachmentCount(1)
            .setPAttachments(&imageViews[i])
            .setWidth(width)
            .setHeight(height)
            .setLayers(1);
        swapchainFramebuffers[i] = device.createFramebuffer(framebufferCi);
    }
}

void RenderContext::createCommandPool(
    vk::Device device,
    uint32_t queueFamily
) {
    commandPool = device.createCommandPool(vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(queueFamily)
    );
}

void RenderContext::createCommandBuffers(
    vk::Device device,
    vk::CommandPool commandPool,
    uint32_t count
) {
    commandBuffers = device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(count)
    );
}

void RenderContext::recordCommandBuffers() {
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    
    for (size_t i = 0; i < commandBuffers.size(); i++) {

        const auto& commandBuffer = commandBuffers[i];
        const auto& framebuffer = swapchainFramebuffers[i];

        // Begin recording command buffer
        auto commandBufferBi = vk::CommandBufferBeginInfo();
        commandBuffer.begin(commandBufferBi);

        auto clearValue = vk::ClearValue()
            .setColor(vk::ClearColorValue()
                .setFloat32({ 0.f, 0.f, 0.f, 1.f })
            );
        auto renderPassBi = vk::RenderPassBeginInfo()
            .setRenderPass(renderPass)
            .setFramebuffer(framebuffer)
            .setRenderArea(
                vk::Rect2D().setExtent(surfaceCapabilities.maxImageExtent)
            )
            .setClearValueCount(1)
            .setPClearValues(&clearValue);

        // COMMAND 0 - Initiate the render pass
        commandBuffer.beginRenderPass(
            renderPassBi, 
            vk::SubpassContents::eInline
        );

        // COMMAND 1 - Bind the pipeline
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        // COMMAND 2 - Bind the vertex buffer
        vk::DeviceSize offset = 0;
        commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);

        // COMMAND 3 - Draw
        commandBuffer.draw(3, 1, 0, 0);

        // COMMAND 4 - End the render pass
        commandBuffer.endRenderPass();

        // End recording command buffer
        commandBuffer.end();
    }
}