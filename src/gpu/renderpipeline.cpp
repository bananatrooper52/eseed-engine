#include "renderpipeline.hpp"

RenderPipeline::RenderPipeline(
    vk::Device device, 
    std::vector<vk::ImageView> imageViews,
    vk::SurfaceFormatKHR surfaceFormat,
    vk::CommandPool commandPool,
    esd::math::Vec2<float> size,
    vk::ShaderModule vertShader,
    vk::ShaderModule fragShader
) : device(device), size(size), commandPool(commandPool) {

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
            .setLayout(layout.layout)
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

RenderPipeline::~RenderPipeline() {
    device.freeCommandBuffers(commandPool, commandBuffers);
    for (auto framebuffer : framebuffers) 
        device.destroyFramebuffer(framebuffer);
    device.destroyPipeline(pipeline);
    device.destroyRenderPass(renderPass);
}

size_t RenderPipeline::registerMeshBuffer(
    std::shared_ptr<MeshBuffer> meshBuffer
) {
    meshBuffers.push_back(meshBuffer);
    recordCommandBuffers();

    return meshBuffers.size() - 1;
}

RenderPipeline::SubpassContainer::SubpassContainer(
    std::vector<vk::AttachmentReference> _attachmentReferences
) {
    attachmentReferences = _attachmentReferences;
    subpass = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount((uint32_t)attachmentReferences.size())
        .setPColorAttachments(attachmentReferences.data());
}

RenderPipeline::VertexInputStateContainer::VertexInputStateContainer(
    std::vector<vk::VertexInputBindingDescription> _bindings,
    std::vector<vk::VertexInputAttributeDescription> _attributes
) {
    bindings = _bindings;
    attributes = _attributes;
    vertexInputState = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount((uint32_t)bindings.size())
        .setPVertexBindingDescriptions(bindings.data())
        .setVertexAttributeDescriptionCount((uint32_t)attributes.size())
        .setPVertexAttributeDescriptions(attributes.data());
}

RenderPipeline::ViewportStateContainer::ViewportStateContainer(
    std::vector<vk::Viewport> _viewports,
    std::vector<vk::Rect2D> _scissors
) {
    viewports = _viewports;
    scissors = _scissors;
    viewportState = vk::PipelineViewportStateCreateInfo()
        .setViewportCount((uint32_t)viewports.size())
        .setPViewports(viewports.data())
        .setScissorCount((uint32_t)scissors.size())
        .setPScissors(scissors.data());
}

RenderPipeline::ColorBlendStateContainer::ColorBlendStateContainer(
    std::vector<vk::PipelineColorBlendAttachmentState> _blendStates
) {
    blendStates = _blendStates;

    colorBlendState = vk::PipelineColorBlendStateCreateInfo()
        .setAttachmentCount((uint32_t)blendStates.size())
        .setPAttachments(blendStates.data());
}

RenderPipeline::LayoutContainer::LayoutContainer(
    vk::Device device,
    std::vector<std::vector<vk::DescriptorSetLayoutBinding>> setLayouts
) : device(device) {

    std::vector<vk::DescriptorSetLayout> setLayoutObjects(setLayouts.size());

    for (size_t i = 0; i < setLayoutObjects.size(); i++) {
        setLayoutObjects[i] = device.createDescriptorSetLayout(
            vk::DescriptorSetLayoutCreateInfo()
                .setBindingCount((uint32_t)setLayouts[i].size())
                .setPBindings(setLayouts[i].data())
        );
    }

    layout = device.createPipelineLayout(vk::PipelineLayoutCreateInfo()
        .setSetLayoutCount((uint32_t)setLayoutObjects.size())
        .setPSetLayouts(setLayoutObjects.data())
    );

    for (size_t i = 0; i < setLayoutObjects.size(); i++) {
        device.destroyDescriptorSetLayout(setLayoutObjects[i]);
    }
}

RenderPipeline::LayoutContainer::~LayoutContainer() {
    device.destroyPipelineLayout(layout);
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

        // Draw each vertex buffer
        for (auto meshBuffer : meshBuffers) {
            // Bind vertex buffer
            vk::DeviceSize offset = 0;
            commandBuffers[i].bindVertexBuffers(
                0, 1, &meshBuffer->buffer, &offset
            );

            // Draw
            // TODO: remove hard-coded vertex count
            commandBuffers[i].draw(meshBuffer->vertexCount, 1, 0, 0);
        }

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
    std::vector<vk::ImageView> imageViews
) {
    std::vector<vk::Framebuffer> swapchainFramebuffers(imageViews.size());

    for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
        swapchainFramebuffers[i] 
            = resourceManager->getDevice().createFramebuffer(
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

std::vector<RenderPipeline::SubpassContainer> RenderPipeline::createSubpasses() 
{
    std::vector<SubpassContainer> subpassInfos;
    
    subpassInfos.push_back(
        SubpassContainer({
            // For attachment 0
            vk::AttachmentReference()
                .setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        })
    );

    return subpassInfos;
}

std::vector<vk::SubpassDescription> RenderPipeline::extractSubpasses(
    std::vector<SubpassContainer> infos
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

RenderPipeline::VertexInputStateContainer 
    RenderPipeline::createVertexInputState() 
{
    return VertexInputStateContainer(
        {
            // 0 - Vertex positions
            vk::VertexInputBindingDescription()
                .setBinding(0)
                .setStride(sizeof(Vertex))
                .setInputRate(vk::VertexInputRate::eVertex)
        },
        {
            // 0 - Main vertex data
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(0)
                .setOffset(offsetof(Vertex, position))
                .setFormat(vk::Format::eR32G32Sfloat),
            
            // 1 - Color data
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(1)
                .setOffset(offsetof(Vertex, color))
                .setFormat(vk::Format::eR32G32B32Sfloat)
        }
    );
}

vk::PipelineInputAssemblyStateCreateInfo
    RenderPipeline::createInputAssemblyState()
{
    return vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList);
}

RenderPipeline::ViewportStateContainer RenderPipeline::createViewportState(
    esd::math::Vec2<float> size
) {
    return ViewportStateContainer(
        { // Viewports
            // 0 - Main viewport, whole screen
            vk::Viewport()
                .setWidth(size.x)
                .setHeight(size.y)
                .setMaxDepth(0)
                .setMaxDepth(1)
        },
        { // Scissors
            // 0 - Main scissor, whole screen
            vk::Rect2D().setExtent({ (uint32_t)size.x, (uint32_t)size.y })
        }
    );
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

RenderPipeline::ColorBlendStateContainer 
    RenderPipeline::createColorBlendState() 
{

    return ColorBlendStateContainer(
        {
            // 0 - Main output color attachment
            vk::PipelineColorBlendAttachmentState()
                .setColorWriteMask(
                    vk::ColorComponentFlagBits::eR |
                    vk::ColorComponentFlagBits::eG |
                    vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA
                )
        }
    );
}

RenderPipeline::LayoutContainer 
    RenderPipeline::createLayout(vk::Device device) 
{
    return LayoutContainer(
        device,
        {
            {
                vk::DescriptorSetLayoutBinding()
                    .setBinding(0)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eVertex)
            }
        }
    );
}