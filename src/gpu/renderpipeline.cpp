#include "renderpipeline.hpp"

RenderPipeline::RenderPipeline(
    std::shared_ptr<ResourceManager> rm,
    std::shared_ptr<PresentManager> pm,
    vk::ShaderModule vertModule,
    vk::ShaderModule fragModule
) : rm(rm), pm(pm) {

    // -- RENDER PASS -- //

    auto mainColorAttachment = vk::AttachmentDescription()
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
        .setFormat(vk::Format::eB8G8R8A8Unorm)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    
    auto mainSubpassColorAttachment = vk::AttachmentReference()
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    
    auto mainSubpass = vk::SubpassDescription()
        .setColorAttachmentCount(1)
        .setPColorAttachments(&mainSubpassColorAttachment)
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    
    renderPass = rm->getDevice().createRenderPass(vk::RenderPassCreateInfo()
        .setAttachmentCount(1)
        .setPAttachments(&mainColorAttachment)
        .setSubpassCount(1)
        .setPSubpasses(&mainSubpass)
    );

    // -- LAYOUT -- //

    layout = rm->getDevice().createPipelineLayout(vk::PipelineLayoutCreateInfo()
        
    );

    // -- PIPELINE -- //

    std::vector<vk::PipelineShaderStageCreateInfo> stages = {
        vk::PipelineShaderStageCreateInfo{
            {},
            vk::ShaderStageFlagBits::eVertex,
            vertModule,
            "main"  
        },
        vk::PipelineShaderStageCreateInfo{
            {},
            vk::ShaderStageFlagBits::eFragment,
            fragModule,
            "main"
        }
    };

    auto mainVertexBinding = vk::VertexInputBindingDescription()
        .setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(sizeof(Vertex));

    auto vertexAttributes = std::vector<vk::VertexInputAttributeDescription>{
        vk::VertexInputAttributeDescription{
            0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)
        },
        vk::VertexInputAttributeDescription{
            1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)
        }
    };

    auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount(1)
        .setPVertexBindingDescriptions(&mainVertexBinding)
        .setVertexAttributeDescriptionCount((uint32_t)vertexAttributes.size())
        .setPVertexAttributeDescriptions(vertexAttributes.data());

    auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList);

    auto viewport = vk::Viewport()
        .setWidth((float)pm->getSize().x)
        .setHeight((float)pm->getSize().y)
        .setMinDepth(0)
        .setMaxDepth(1);

    auto scissor = vk::Rect2D()
        .setExtent({ pm->getSize().x, pm->getSize().y });

    auto viewportState = vk::PipelineViewportStateCreateInfo()
        .setViewportCount(1)
        .setPViewports(&viewport)
        .setScissorCount(1)
        .setPScissors(&scissor);

    auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
        .setPolygonMode(vk::PolygonMode::eFill)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setLineWidth(1.f);

    auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    auto mainColorBlendAttachment = vk::PipelineColorBlendAttachmentState()
        .setColorWriteMask(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        );

    auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
        .setAttachmentCount(1)
        .setPAttachments(&mainColorBlendAttachment);

    pipeline = rm->getDevice().createGraphicsPipeline(
        nullptr, 
        vk::GraphicsPipelineCreateInfo()
        .setStageCount((uint32_t)stages.size())
        .setPStages(stages.data())
        .setPVertexInputState(&vertexInputState)
        .setPInputAssemblyState(&inputAssemblyState)
        .setPViewportState(&viewportState)
        .setPRasterizationState(&rasterizationState)
        .setPMultisampleState(&multisampleState)
        .setPColorBlendState(&colorBlendState)
        .setLayout(layout)
        .setRenderPass(renderPass)
        .setSubpass(0)
    );

    // -- COMMAND POOL -- //

    commandPool = rm->getDevice().createCommandPool(vk::CommandPoolCreateInfo()
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(*rm->getGraphicsQueueFamily())
    );

    // -- COMMAND BUFFERS -- //

    commandBuffers = rm->getDevice().allocateCommandBuffers(
        vk::CommandBufferAllocateInfo()
        .setCommandBufferCount(pm->getImageCount())
        .setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
    );

    // -- FRAMEBUFFERS -- //

    framebuffers.resize(pm->getImageCount());
    for (uint32_t i = 0; i < pm->getImageCount(); i++) {
        framebuffers[i] = rm->getDevice().createFramebuffer(
            vk::FramebufferCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&pm->getImageView(i))
            .setWidth(pm->getSize().x)
            .setHeight(pm->getSize().y)
            .setRenderPass(renderPass)
            .setLayers(1)
        );
    }
}

RenderPipeline::~RenderPipeline() {
    for (const auto& framebuffer : framebuffers)
        rm->getDevice().destroyFramebuffer(framebuffer);
    rm->getDevice().freeCommandBuffers(commandPool, commandBuffers);
    rm->getDevice().destroyCommandPool(commandPool);
    rm->getDevice().destroyPipeline(pipeline);
    rm->getDevice().destroyPipelineLayout(layout);
    rm->getDevice().destroyRenderPass(renderPass);
}

RenderObject::Id RenderPipeline::addRenderObject(const Mesh& mesh) {
    RenderObject object;

    vk::DeviceSize byteLength = sizeof(mesh.vertices[0]) * mesh.vertices.size();
    
    object.vertexCount = (uint32_t)mesh.vertices.size();

    auto queueFamily = *rm->getGraphicsQueueFamily();
    object.vertexBuffer = rm->getDevice().createBuffer(vk::BufferCreateInfo()
        .setQueueFamilyIndexCount(1)
        .setPQueueFamilyIndices(&queueFamily)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSize(byteLength)
        .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
    );
    
    // Find memory type index
    auto memoryProperties = rm->getPhysicalDevice().getMemoryProperties();
    auto memoryRequirements = 
        rm->getDevice().getBufferMemoryRequirements(object.vertexBuffer);
    auto properties = 
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent;

    std::optional<uint32_t> memoryTypeIndex;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) && 
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) 
            == properties))
            memoryTypeIndex = i;
    }
    if (!memoryTypeIndex) 
        throw std::runtime_error("Could not find suitable memory type index");

    object.memory = rm->getDevice().allocateMemory(vk::MemoryAllocateInfo()
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(*memoryTypeIndex)
    );

    rm->getDevice().bindBufferMemory(object.vertexBuffer, object.memory, 0);

    void* data = rm->getDevice().mapMemory(object.memory, 0, byteLength);
    memcpy(data, mesh.vertices.data(), byteLength);
    rm->getDevice().unmapMemory(object.memory);

    RenderObject::Id id = 0;
    while (renderObjects.count(id)) id++;
    renderObjects[id] = object;

    return id;
}

void RenderPipeline::removeRenderObject(RenderObject::Id id) {
    auto object = renderObjects[id];

    rm->getDevice().freeMemory(object.memory);
    rm->getDevice().destroyBuffer(object.vertexBuffer);
    
    renderObjects.erase(id);
}

RenderInstance::Id RenderPipeline::addRenderInstance(
    RenderObject::Id objectId
) {
    RenderInstance::Id id = 0;
    while (renderInstances.count(id)) id++;
    renderInstances[id] = { objectId };

    recordCommandBuffers();

    return id;    
}

void RenderPipeline::removeRenderInstance(RenderInstance::Id id) {
    renderInstances.erase(id);
    recordCommandBuffers();
}

vk::CommandBuffer RenderPipeline::getCommandBuffer(uint32_t i) {
    return commandBuffers[i];
}

void RenderPipeline::recordCommandBuffers() {
    for (uint32_t i = 0; i < commandBuffers.size(); i++) {
        vk::CommandBuffer& cmd = commandBuffers[i];

        // Begin recording
        cmd.begin(vk::CommandBufferBeginInfo());

        // Begin render pass
        vk::ClearValue clearValue = vk::ClearValue().setColor(
            vk::ClearColorValue().setFloat32({ 0.f, 0.f, 0.f, 1.f })
        );
        cmd.beginRenderPass(
            vk::RenderPassBeginInfo()
            .setClearValueCount(1)
            .setPClearValues(&clearValue)
            .setFramebuffer(framebuffers[i])
            .setRenderPass(renderPass)
            .setRenderArea({{ 0, 0 }, { pm->getSize().x, pm->getSize().y }}),
            vk::SubpassContents::eInline
        );

        // Bind the pipeline
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        // For each render instance, bind vertex buffer and draw
        for (const auto& renderInstance : renderInstances) {

            const auto& renderObject = 
                renderObjects[renderInstance.second.objectId];
            
            cmd.bindVertexBuffers(
                0, 
                { renderObject.vertexBuffer },
                { 0 }
            );

            cmd.draw(renderObject.vertexCount, 1, 0, 0);
        }

        // End render pass
        cmd.endRenderPass();

        // End recording
        cmd.end();
    }
}