#pragma once

#include "resourcemanager.hpp"
#include "presentmanager.hpp"
#include "mesh.hpp"

#include <vulkan/vulkan.hpp>
#include <map>

struct RenderObject {
    using Id = size_t;
    
    vk::DeviceMemory memory;
    vk::Buffer vertexBuffer;

    uint32_t vertexCount;
};

struct RenderInstance {
    using Id = size_t;
    
    RenderObject::Id objectId;
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

    std::map<RenderObject::Id, RenderObject> renderObjects;
    std::map<RenderInstance::Id, RenderInstance> renderInstances;

    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
    vk::PipelineLayout layout;
    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<vk::Framebuffer> framebuffers;

    void recordCommandBuffers();
};