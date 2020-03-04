#include "gpu.hpp"

#include <fstream>

Gpu::Gpu(std::shared_ptr<esd::window::Window> window) {

    // Create instance
    instance = createInstance(true, window);
    
    // Create surface if window is provided
    surface = window->createSurface(instance);

    // Find physical device
    physicalDevice = findPhysicalDevice(instance);

    // Create device
    auto deviceInfo = createDevice(instance, physicalDevice, surface);
    device = deviceInfo.device;

    // Extract queues
    graphicsQueue = device.getQueue(*deviceInfo.graphicsQueueFamily, 0);

    vk::SurfaceFormatKHR surfaceFormat;
    for (auto format : physicalDevice.getSurfaceFormatsKHR(surface)) {
        surfaceFormat = format;
        break;
    }

    // Create swapchain
    swapchain = createSwapchain(
        physicalDevice, 
        device, 
        surface,
        surfaceFormat
    );

    swapchainImageViews = createSwapchainImageViews(
        device, 
        swapchain, 
        surfaceFormat.format
    );

    auto surfaceCapabilities = 
        physicalDevice.getSurfaceCapabilitiesKHR(surface);

    renderPass = createRenderPass(
        device, 
        surfaceFormat
    );

    pipeline = createRenderPipeline(
        device,
        renderPass,
        loadShaderCode("./resources/shaders/test/test.vert.spv"),
        loadShaderCode("./resources/shaders/test/test.frag.spv"),
        (float)surfaceCapabilities.maxImageExtent.width,
        (float)surfaceCapabilities.maxImageExtent.height
    );

    swapchainFramebuffers = createSwapchainFramebuffers(
        device, 
        swapchain, 
        renderPass, 
        swapchainImageViews,
        surfaceCapabilities.maxImageExtent.width,
        surfaceCapabilities.maxImageExtent.height
    );

    commandPool = createCommandPool(device, *deviceInfo.graphicsQueueFamily);

    commandBuffers = createCommandBuffers(
        device, 
        commandPool, 
        (uint32_t)swapchainImageViews.size()
    );

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

        // COMMAND 2 - Draw
        commandBuffer.draw(3, 1, 0, 0);

        // COMMAND 3 - End the render pass
        commandBuffer.endRenderPass();

        // End recording command buffer
        commandBuffer.end();
    }

    imageAvailableSemaphore = device.createSemaphore({});
    renderFinishedSemaphore = device.createSemaphore({});
}

std::vector<uint8_t> Gpu::loadShaderCode(std::string path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) throw std::runtime_error("File does not exist");

    std::vector<uint8_t> code;

    file.seekg(0, file.end);
    code.resize(file.tellg());
    file.seekg(0, file.beg);

    file.read((char*)code.data(), code.size());

    return code;
}

vk::Instance Gpu::createInstance(
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
    
    auto instanceCi = vk::InstanceCreateInfo()
        .setEnabledExtensionCount((uint32_t)extensions.size())
        .setPpEnabledExtensionNames(extensions.data())
        .setEnabledLayerCount((uint32_t)layers.size())
        .setPpEnabledLayerNames(layers.data());
    
    return vk::createInstance(instanceCi);
}

vk::PhysicalDevice Gpu::findPhysicalDevice(vk::Instance instance) {
    // TODO: do a little more thinking than just picking the first option
    return instance.enumeratePhysicalDevices()[0];
}

Gpu::DeviceInfo Gpu::createDevice(
    vk::Instance instance, 
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR surface
) {
    // Locate queue families
    std::optional<uint32_t> graphicsQueueFamily;
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
    
    auto deviceCi = vk::DeviceCreateInfo()
        .setQueueCreateInfoCount((uint32_t)queueCis.size())
        .setPQueueCreateInfos(queueCis.data())
        .setEnabledExtensionCount((uint32_t)extensions.size())
        .setPpEnabledExtensionNames(extensions.data());

    return DeviceInfo {
        physicalDevice.createDevice(deviceCi),
        graphicsQueueFamily
    };
}

vk::SwapchainKHR Gpu::createSwapchain(
    vk::PhysicalDevice physicalDevice,
    vk::Device device,
    vk::SurfaceKHR surface,
    vk::SurfaceFormatKHR surfaceFormat
) {
    auto surfaceCapabilities = 
        physicalDevice.getSurfaceCapabilitiesKHR(surface);
    
    auto swapchainCi = vk::SwapchainCreateInfoKHR()
        .setSurface(surface)
        .setMinImageCount(surfaceCapabilities.minImageCount)
        .setImageExtent(surfaceCapabilities.maxImageExtent)
        .setImageFormat(surfaceFormat.format)
        .setPreTransform(
            physicalDevice.getSurfaceCapabilitiesKHR(surface).currentTransform
        )
        .setPresentMode(vk::PresentModeKHR::eFifo)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageArrayLayers(1)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setClipped(true);

    return device.createSwapchainKHR(swapchainCi);
}

std::vector<vk::ImageView> Gpu::createSwapchainImageViews(
    vk::Device device,
    vk::SwapchainKHR swapchain,
    vk::Format format
) {
    auto images = device.getSwapchainImagesKHR(swapchain);

    std::vector<vk::ImageView> imageViews(images.size());
    for (size_t i = 0; i < imageViews.size(); i++) {
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
        imageViews[i] = device.createImageView(imageViewCi);
    }
    
    return imageViews;
}

vk::RenderPass Gpu::createRenderPass(
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

    auto renderPassCi = vk::RenderPassCreateInfo()
        .setAttachmentCount(1)
        .setPAttachments(&colorAttachmentDesc)
        .setSubpassCount(1)
        .setPSubpasses(&subpassDesc);

    return device.createRenderPass(renderPassCi);
}

vk::Pipeline Gpu::createRenderPipeline(
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

    // TODO: No vertices inputted currently
    auto vertexInputStateCi = vk::PipelineVertexInputStateCreateInfo();

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

    auto graphicsPipelineCi = vk::GraphicsPipelineCreateInfo()
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
        .setLayout(pipelineLayout);

    return device.createGraphicsPipeline(nullptr, graphicsPipelineCi);
}

void Gpu::render() {
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

vk::ShaderModule Gpu::createShaderModule(
    vk::Device device,
    const std::vector<uint8_t>& code
) {
    auto shaderModuleCi = vk::ShaderModuleCreateInfo()
        .setCodeSize(code.size())
        .setPCode((uint32_t*)code.data());

    return device.createShaderModule(shaderModuleCi);
}

std::vector<vk::Framebuffer> Gpu::createSwapchainFramebuffers(
    vk::Device device,
    vk::SwapchainKHR swapchain,
    vk::RenderPass renderPass,
    std::vector<vk::ImageView> imageViews,
    uint32_t width,
    uint32_t height
) {
    std::vector<vk::Framebuffer> framebuffers(imageViews.size());

    for (size_t i = 0; i < framebuffers.size(); i++) {
        auto framebufferCi = vk::FramebufferCreateInfo()
            .setRenderPass(renderPass)
            .setAttachmentCount(1)
            .setPAttachments(&imageViews[i])
            .setWidth(width)
            .setHeight(height)
            .setLayers(1);
        framebuffers[i] = device.createFramebuffer(framebufferCi);
    }

    return framebuffers;
}

vk::CommandPool Gpu::createCommandPool(
    vk::Device device,
    uint32_t queueFamily
) {
    auto commandPoolCi = vk::CommandPoolCreateInfo()
        .setQueueFamilyIndex(queueFamily);

    return device.createCommandPool(commandPoolCi);
}

std::vector<vk::CommandBuffer> Gpu::createCommandBuffers(
    vk::Device device,
    vk::CommandPool commandPool,
    uint32_t count
) {
    auto commandBufferAi = vk::CommandBufferAllocateInfo()
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(count);

    return device.allocateCommandBuffers(commandBufferAi);
}