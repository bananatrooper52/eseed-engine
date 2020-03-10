#pragma once

#include "resourcemanager.hpp"
#include "presentmanager.hpp"
#include "mesh.hpp"

#include <vulkan/vulkan.hpp>
#include <map>

#define ALIGN_SCALAR alignas(4)
#define ALIGN_VEC2 alignas(8)
#define ALIGN_VEC4 alignas(16)
#define ALIGN_NESTED_STRUCT alignas(16)
#define ALIGN_MAT4 alignas(ALIGN_VEC4)

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
    ALIGN_SCALAR float x;
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

    void setCamera(const Camera& camera);

    void update(uint32_t imageIndex);

private:
    Camera camera;

    std::shared_ptr<ResourceManager> rm;
    std::shared_ptr<PresentManager> pm;

    std::vector<MemoryContainer> cameraMemoryContainers;
    std::map<RenderObject::Id, RenderObject> renderObjects;
    std::map<RenderInstance::Id, RenderInstance> renderInstances;

    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
    vk::PipelineLayout layout;
    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<vk::Framebuffer> framebuffers;
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    void recordCommandBuffers();

    MemoryContainer createMemoryContainer(
        std::vector<std::pair<size_t, vk::BufferUsageFlags>> bufferSizes
    );
    void destroyMemoryContainer(const MemoryContainer& container);
    void setBufferData(
        const MemoryContainer& container, 
        size_t bufferIndex, 
        const void* data,
        size_t size
    );
};