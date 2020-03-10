#pragma once

#include "resourcemanager.hpp"
#include "presentmanager.hpp"
#include "mesh.hpp"

#include <vulkan/vulkan.hpp>
#include <map>

struct MemoryContainer {
    vk::DeviceMemory memory;
    std::vector<vk::Buffer> buffers;
};

struct RenderObject {
    using Id = size_t;

    MemoryContainer memoryContainer;
    uint32_t vertexCount;
};

struct RenderInstance {
    using Id = size_t;
    
    RenderObject::Id objectId;
};

struct Camera {
    float x;
};

class RenderPipeline {
public:
    RenderPipeline::RenderPipeline(
        std::shared_ptr<ResourceManager> rm,
        std::shared_ptr<PresentManager> pm,
        vk::ShaderModule vertModule,
        vk::ShaderModule fragModule
    );
    ~RenderPipeline();

    RenderObject::Id addRenderObject(const Mesh& mesh);
    void removeRenderObject(RenderObject::Id id);

    RenderInstance::Id addRenderInstance(RenderObject::Id objectId);
    void removeRenderInstance(RenderInstance::Id id);

    vk::CommandBuffer getCommandBuffer(uint32_t i);

private:
    std::shared_ptr<ResourceManager> rm;
    std::shared_ptr<PresentManager> pm;

    MemoryContainer cameraMemoryContainer;
    std::map<RenderObject::Id, RenderObject> renderObjects;
    std::map<RenderInstance::Id, RenderInstance> renderInstances;

    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
    vk::PipelineLayout layout;
    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<vk::Framebuffer> framebuffers;

    void recordCommandBuffers();

    MemoryContainer createMemoryContainer(std::vector<size_t> bufferSizes);
    void destroyMemoryContainer(const MemoryContainer& container);
    void setBufferData(
        const MemoryContainer& container, 
        size_t bufferIndex, 
        const void* data,
        size_t size
    );
};