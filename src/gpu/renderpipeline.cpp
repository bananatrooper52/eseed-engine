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

    auto cameraSetLayoutBinding = vk::DescriptorSetLayoutBinding()
        .setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    auto cameraSetLayout = rm->getDevice().createDescriptorSetLayout(
        vk::DescriptorSetLayoutCreateInfo()
        .setBindingCount(1)
        .setPBindings(&cameraSetLayoutBinding)
    );

    layout = rm->getDevice().createPipelineLayout(vk::PipelineLayoutCreateInfo()
        .setSetLayoutCount(1)
        .setPSetLayouts(&cameraSetLayout)
    );

    rm->getDevice().destroyDescriptorSetLayout(cameraSetLayout);

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

    cameraMemoryContainer = createMemoryContainer({ sizeof(Camera) });

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
    size_t byteLength = mesh.vertices.size() * sizeof(mesh.vertices[0]);

    RenderObject object = {
        createMemoryContainer({ byteLength }),
        (uint32_t)mesh.vertices.size()
    };
    
    object.vertexCount = (uint32_t)mesh.vertices.size();

    setBufferData(object.memoryContainer, 0, mesh.vertices.data(), byteLength);

    RenderObject::Id id = 0;
    while (renderObjects.count(id)) id++;
    renderObjects[id] = object;

    return id;
}

void RenderPipeline::removeRenderObject(RenderObject::Id id) {
    auto object = renderObjects[id];

    destroyMemoryContainer(renderObjects[id].memoryContainer);
    
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
                { renderObject.memoryContainer.buffers[0] },
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

MemoryContainer RenderPipeline::createMemoryContainer(
    std::vector<size_t> bufferSizes
) {
    MemoryContainer container;
    
    auto queueFamily = *rm->getGraphicsQueueFamily();

    // Create buffers and find offsets

    vk::DeviceSize totalMemorySize = 0;

    container.buffers.resize(bufferSizes.size());
    std::vector<vk::DeviceSize> bufferOffsets(container.buffers.size());

    uint32_t typeBits = 0;
    
    for (size_t i = 0; i < container.buffers.size(); i++) {
        container.buffers[i] = rm->getDevice().createBuffer(
            vk::BufferCreateInfo()
            .setQueueFamilyIndexCount(1)
            .setPQueueFamilyIndices(&queueFamily)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSize(bufferSizes[i])
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
        );

        auto memReqs = rm->getDevice().getBufferMemoryRequirements(
            container.buffers[i]
        );

        bufferOffsets[i] = totalMemorySize;
        
        typeBits |= memReqs.memoryTypeBits;

        totalMemorySize += memReqs.size;
    }
    
    // Find memory type index
    auto memProps = rm->getPhysicalDevice().getMemoryProperties();
    auto properties = 
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent;

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        bool hasAllTypeBits = (typeBits & (1 << i));
        bool hasAllProperties = 
            (memProps.memoryTypes[i].propertyFlags & properties) == properties;
            
        if (hasAllTypeBits && hasAllProperties) {
            memoryTypeIndex = i;
            break;
        }
    }

    // Allocate memory
    container.memory = rm->getDevice().allocateMemory(vk::MemoryAllocateInfo()
        .setAllocationSize(totalMemorySize)
        .setMemoryTypeIndex(memoryTypeIndex)
    );

    // Bind buffers to memory
    for (size_t i = 0; i < container.buffers.size(); i++) {
        rm->getDevice().bindBufferMemory(
            container.buffers[i],
            container.memory,
            bufferOffsets[i]
        );
    }

    return container;
}

void RenderPipeline::destroyMemoryContainer(const MemoryContainer& container) {
    for (const auto& buffer : container.buffers)
        rm->getDevice().destroyBuffer(buffer);

    rm->getDevice().freeMemory(container.memory);
}

void RenderPipeline::setBufferData(
    const MemoryContainer& container,
    size_t bufferIndex,
    const void* data,
    size_t size
) {
    void* mappedData = rm->getDevice().mapMemory(
        container.memory, 
        0, 
        size
    );
    memcpy(mappedData, data, size);
    rm->getDevice().unmapMemory(container.memory);
}