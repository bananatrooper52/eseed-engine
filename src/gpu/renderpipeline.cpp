#include "renderpipeline.hpp"

RenderPipeline::RenderPipeline(
    vk::Device device, 
    std::vector<vk::ImageView> imageViews,
    vk::SurfaceFormatKHR surfaceFormat,
    vk::CommandPool commandPool,
    esd::math::Vec2<float> size,
    vk::ShaderModule vertShader,
    vk::ShaderModule fragShader
) : device(device), size(size) {

    auto renderPassAttachments = createAttachments(surfaceFormat);
    auto renderPassSubpassInfos = createSubpasses();
    auto renderPassSubpasses = extractSubpasses(renderPassSubpassInfos);
    
    renderPass = device.createRenderPass(vk::RenderPassCreateInfo()
            .setAttachmentCount((uint32_t)renderPassAttachments.size())
            .setPAttachments(renderPassAttachments.data())
            .setSubpassCount((uint32_t)renderPassSubpasses.size())
            .setPSubpasses(renderPassSubpasses.data())
    );

    auto stages = createStages(vertShader, fragShader);
    auto vertexInputStateInfo = createVertexInputState();
    auto inputAssemblyState = createInputAssemblyState();
    auto viewportStateInfo = createViewportState(size);
    auto rasterizationState = createRasterizationState();
    auto multisampleState = createMultisampleState();
    auto colorBlendStateInfo = createColorBlendState();
    auto layout = createLayout(device);

    pipeline = device.createGraphicsPipeline(
        nullptr,
        vk::GraphicsPipelineCreateInfo()
            .setStageCount((uint32_t)stages.size())
            .setPStages(stages.data())
            .setPVertexInputState(&vertexInputStateInfo.vertexInputState)
            .setPInputAssemblyState(&inputAssemblyState)
            .setPViewportState(&viewportStateInfo.viewportState)
            .setPRasterizationState(&rasterizationState)
            .setPMultisampleState(&multisampleState)
            .setPColorBlendState(&colorBlendStateInfo.colorBlendState)
            .setLayout(layout)
            .setRenderPass(renderPass)
            .setSubpass(0)
    );

    framebuffers = createFramebuffers(device, renderPass, imageViews, size);

    commandBuffers = createCommandBuffers(
        device, 
        commandPool, 
        (uint32_t)imageViews.size()
    );
}

void RenderPipeline::addVertexBuffer(vk::Buffer buffer) {
    vertexBuffer = buffer;
    recordCommandBuffers();
}

void RenderPipeline::recordCommandBuffers() {
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        commandBuffers[i].begin(vk::CommandBufferBeginInfo());

        // Begin render pass
        std::vector<vk::ClearValue> clearValues;
        clearValues.push_back(vk::ClearValue()
            .setColor(vk::ClearColorValue().setFloat32({ 0.f, 0.f, 0.f, 1.f }))
        ); 
        commandBuffers[i].beginRenderPass(vk::RenderPassBeginInfo()
                .setClearValueCount((uint32_t)clearValues.size())
                .setPClearValues(clearValues.data())
                .setFramebuffer(framebuffers[i])
                .setRenderPass(renderPass)
                .setRenderArea(vk::Rect2D().setExtent(
                    {(uint32_t)size.x, (uint32_t)size.y}
                )),
            vk::SubpassContents::eInline
        );

        // Bind pipeline
        commandBuffers[i].bindPipeline(
            vk::PipelineBindPoint::eGraphics, 
            pipeline
        );

        // Bind vertex buffer
        vk::DeviceSize offset = 0;
        commandBuffers[i].bindVertexBuffers(0, 1, &vertexBuffer, &offset);

        // Draw
        // TODO: remove hard-coded vertex count
        commandBuffers[i].draw(3, 1, 0, 0);

        // End render pass
        commandBuffers[i].endRenderPass();

        commandBuffers[i].end();
    }
}

std::vector<vk::CommandBuffer> RenderPipeline::createCommandBuffers(
    vk::Device device,
    vk::CommandPool commandPool,
    uint32_t count
) { 
    return device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo()
            .setCommandBufferCount(count)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandPool(commandPool)
    );
}

std::vector<vk::Framebuffer> RenderPipeline::createFramebuffers(
    vk::Device device,
    vk::RenderPass renderPass,
    std::vector<vk::ImageView> imageViews,
    esd::math::Vec2<float> size
) {
    std::vector<vk::Framebuffer> swapchainFramebuffers(imageViews.size());

    for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
        swapchainFramebuffers[i] = device.createFramebuffer(
            vk::FramebufferCreateInfo()
                .setRenderPass(renderPass)
                .setAttachmentCount(1)
                .setPAttachments(&imageViews[i])
                .setWidth((uint32_t)size.x)
                .setHeight((uint32_t)size.y)
                .setLayers(1)
        );
    }

    return swapchainFramebuffers;
}

std::vector<vk::AttachmentDescription> 
RenderPipeline::createAttachments(vk::SurfaceFormatKHR surfaceFormat) {
    return {
        // Attachment 0 - Main output color attachment
        vk::AttachmentDescription()
            .setFormat(surfaceFormat.format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
    };
}

std::vector<RenderPipeline::SubpassInfo> RenderPipeline::createSubpasses() 
{
    std::vector<SubpassInfo> subpassInfos(1);

    subpassInfos[0].attachmentReferences = {
        // For attachment 0
        vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
    };
    subpassInfos[0].subpass = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(
            (uint32_t)subpassInfos[0].attachmentReferences.size()
        )
        .setPColorAttachments(subpassInfos[0].attachmentReferences.data());

    return subpassInfos;
}

std::vector<vk::SubpassDescription> RenderPipeline::extractSubpasses(
    std::vector<SubpassInfo> infos
) {
    std::vector<vk::SubpassDescription> out(infos.size());
    for (size_t i = 0; i < out.size(); i++) {
        out[i] = infos[i].subpass;
    }

    return out;
}

std::vector<vk::PipelineShaderStageCreateInfo> RenderPipeline::createStages(
    vk::ShaderModule vertShader,
    vk::ShaderModule fragShader
) {
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    
    // Vertex
    stages.push_back(vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setPName("main")
            .setModule(vertShader)
    );

    // Fragment
    stages.push_back(vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setPName("main")
            .setModule(fragShader)
    );

    return stages;
}

RenderPipeline::VertexInputStateInfo RenderPipeline::createVertexInputState() {
    VertexInputStateInfo info;
    
    info.bindings = {
        // 0 - Vertex positions
        vk::VertexInputBindingDescription()
            .setBinding(0)
            .setStride(sizeof(esd::math::Vec2<float>))
            .setInputRate(vk::VertexInputRate::eVertex)
    };

    info.attributes = {
        // 0 - Main vertex data
        vk::VertexInputAttributeDescription()
            .setBinding(0)
            .setLocation(0)
            .setOffset(0)
            .setFormat(vk::Format::eR32G32Sfloat)
    };

    info.vertexInputState = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount((uint32_t)info.bindings.size())
        .setPVertexBindingDescriptions(info.bindings.data())
        .setVertexAttributeDescriptionCount((uint32_t)info.attributes.size())
        .setPVertexAttributeDescriptions(info.attributes.data());

    return info;
}

vk::PipelineInputAssemblyStateCreateInfo
    RenderPipeline::createInputAssemblyState()
{
    return vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList);
}

RenderPipeline::ViewportStateInfo RenderPipeline::createViewportState(
    esd::math::Vec2<float> size
) {
    ViewportStateInfo info;
    
    info.viewports = {
        // 0 - Main viewport, whole screen
        vk::Viewport()
            .setWidth(size.x)
            .setHeight(size.y)
            .setMaxDepth(0)
            .setMaxDepth(1)
    };

    info.scissors = {
        // 0 - Main scissor, whole screen
        vk::Rect2D().setExtent({ (uint32_t)size.x, (uint32_t)size.y })
    };
    
    info.viewportState = vk::PipelineViewportStateCreateInfo()
        .setViewportCount((uint32_t)info.viewports.size())
        .setPViewports(info.viewports.data())
        .setScissorCount((uint32_t)info.scissors.size())
        .setPScissors(info.scissors.data());
    
    return info;
}

vk::PipelineRasterizationStateCreateInfo 
    RenderPipeline::createRasterizationState()
{
    return vk::PipelineRasterizationStateCreateInfo()
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise);
}

vk::PipelineMultisampleStateCreateInfo 
    RenderPipeline::createMultisampleState()
{
    return vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);
}

RenderPipeline::ColorBlendStateInfo RenderPipeline::createColorBlendState() {

    ColorBlendStateInfo info;

    info.blendStates = {
        // 0 - Main output color attachment
        vk::PipelineColorBlendAttachmentState()
            .setColorWriteMask(
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
            )
    };

    info.colorBlendState = vk::PipelineColorBlendStateCreateInfo()
        .setAttachmentCount((uint32_t)info.blendStates.size())
        .setPAttachments(info.blendStates.data());

    return info;
}

vk::PipelineLayout RenderPipeline::createLayout(vk::Device device) {
    // TODO: no uniforms
    return device.createPipelineLayout(vk::PipelineLayoutCreateInfo());
}