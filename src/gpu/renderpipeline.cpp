#include "renderpipeline.hpp"

RenderPipeline::RenderPipeline(
    vk::Device device, 
    std::vector<vk::Framebuffer> framebuffers,
    vk::SurfaceFormatKHR surfaceFormat,
    vk::CommandPool commandPool,
    esd::math::Vec2<float> size,
    vk::ShaderModule vertShader,
    vk::ShaderModule fragShader
) : device(device), size(size) {

    auto renderPassAttachments = createRenderPassAttachments(surfaceFormat);
    auto renderPassSubpasses = createRenderPassSubpasses();
    
    renderPass = device.createRenderPass(vk::RenderPassCreateInfo()
            .setAttachmentCount((uint32_t)renderPassAttachments.size())
            .setPAttachments(renderPassAttachments.data())
            .setSubpassCount((uint32_t)renderPassSubpasses.size())
            .setPSubpasses(renderPassSubpasses.data())
    );

    auto pipelineStages = createPipelineStages(vertShader, fragShader);
    auto pipelineVertexInputState = createPipelineVertexInputState();
    auto pipelineInputAssemblyState = createPipelineInputAssemblyState();
    auto pipelineViewportState = createPipelineViewportState();
    auto pipelineRasterizationState = createPipelineRasterizationState();
    auto pipelineMultisampleState = createPipelineMultisampleState();
    auto pipelineColorBlendState = createPipelineColorBlendState();
    auto pipelineLayout = createPipelineLayout();

    pipeline = device.createGraphicsPipeline(
        nullptr,
        vk::GraphicsPipelineCreateInfo()
            .setStageCount((uint32_t)pipelineStages.size())
            .setPStages(pipelineStages.data())
            .setPVertexInputState(&pipelineVertexInputState)
            .setPInputAssemblyState(&pipelineInputAssemblyState)
            .setPViewportState(&pipelineViewportState)
            .setPRasterizationState(&pipelineRasterizationState)
            .setPMultisampleState(&pipelineMultisampleState)
            .setPColorBlendState(&pipelineColorBlendState)
            .setLayout(pipelineLayout)
            .setRenderPass(renderPass)
            .setSubpass(0)
    );

    commandBuffers = createCommandBuffers(device, framebuffers, commandPool);
}

void RenderPipeline::addVertexBuffer(vk::Buffer buffer) {
    vertexBuffer = buffer;
    recordCommandBuffers();
}

void RenderPipeline::recordCommandBuffers() {
    for (size_t i = 0; i < framebuffers.size(); i++) {
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
                .setRenderArea(vk::Rect2D().setExtent({size.x, size.y})),
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
    std::vector<vk::Framebuffer> framebuffers,
    vk::CommandPool commandPool
) { 
    std::vector<vk::CommandBuffer> commandBuffers(framebuffers.size());
    
    for (size_t i = 0; i < framebuffers.size(); i++) {
        commandBuffers[i] = device.allocateCommandBuffers(
            vk::CommandBufferAllocateInfo()
                .setCommandBufferCount(0)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandPool(commandPool)
        );
    }
}

std::vector<vk::AttachmentDescription> 
RenderPipeline::createRenderPassAttachments(
    vk::SurfaceFormatKHR surfaceFormat
) {
    std::vector<vk::AttachmentDescription> attachments;

    // 0 - Main output color attachment
    attachments.push_back(vk::AttachmentDescription()
            .setFormat(surfaceFormat.format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
    );
}

std::vector<vk::SubpassDescription> 
RenderPipeline::createRenderPassSubpasses() {
    std::vector<vk::SubpassDescription> subpasses;

    // 0 - Main render subpass
    std::vector<vk::AttachmentReference> subpass0AttachmentReferences = {
        vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
    };
    subpasses.push_back(vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(
                (uint32_t)subpass0AttachmentReferences.size()
            )
            .setPColorAttachments(subpass0AttachmentReferences.data())
    );

    return subpasses;
}

std::vector<vk::PipelineShaderStageCreateInfo> 
    RenderPipeline::createPipelineStages
(
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

vk::PipelineVertexInputStateCreateInfo
    RenderPipeline::createPipelineVertexInputState() 
{

    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    
    // 0 - Vertex positions
    vertexBindingDescriptions.push_back(vk::VertexInputBindingDescription()
        .setBinding(0)
        .setStride(sizeof(esd::math::Vec2<float>))
        .setInputRate(vk::VertexInputRate::eVertex)
    );
        
    std::vector<vk::VertexInputAttributeDescription> 
        vertexAttributeDescriptions;

    // 0 - Main vertex data
    vertexAttributeDescriptions.push_back(vk::VertexInputAttributeDescription()
        .setBinding(0)
        .setLocation(0)
        .setOffset(0)
        .setFormat(vk::Format::eR32G32Sfloat)
    );

    return vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount(
            (uint32_t)vertexBindingDescriptions.size()
        )
        .setPVertexBindingDescriptions(vertexBindingDescriptions.data())
        .setVertexAttributeDescriptionCount(
            (uint32_t)vertexAttributeDescriptions.size()
        )
        .setPVertexAttributeDescriptions(vertexAttributeDescriptions.data());
}

vk::PipelineInputAssemblyStateCreateInfo
    RenderPipeline::createPipelineInputAssemblyState()
{
    return vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList);
}

vk::PipelineViewportStateCreateInfo
    RenderPipeline::createPipelineViewportState(esd::math::Vec2<float> size)
{
    std::vector<vk::Viewport> viewports;
    viewports.push_back(vk::Viewport()
        .setWidth(size.x)
        .setHeight(size.y)
        .setMaxDepth(0)
        .setMaxDepth(1)
    );

    std::vector<vk::Rect2D> scissors;
    scissors.push_back(vk::Rect2D().setExtent({ size.x, size.y }));
    
    return vk::PipelineViewportStateCreateInfo()
        .setViewportCount((uint32_t)viewports.size())
        .setPViewports(viewports.data())
        .setScissorCount((uint32_t)scissors.size())
        .setPScissors(scissors.data());
}

vk::PipelineRasterizationStateCreateInfo
    RenderPipeline::createPipelineRasterizationState()
{
    return vk::PipelineRasterizationStateCreateInfo()
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise);
}

vk::PipelineMultisampleStateCreateInfo
    RenderPipeline::createPipelineMultisampleState()
{
    return vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);
}

vk::PipelineColorBlendStateCreateInfo
    RenderPipeline::createPipelineColorBlendState()
{
    std::vector<vk::PipelineColorBlendAttachmentState> attachments;
    
    // 0 - Main output color attachment
    attachments.push_back(vk::PipelineColorBlendAttachmentState()
        .setColorWriteMask(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        )
    );

    return vk::PipelineColorBlendStateCreateInfo()
        .setAttachmentCount((uint32_t)attachments.size())
        .setPAttachments(attachments.data());
}

vk::PipelineLayout RenderPipeline::createPipelineLayout(vk::Device device) {
    // TODO: no uniforms
    return device.createPipelineLayout(vk::PipelineLayoutCreateInfo());
}