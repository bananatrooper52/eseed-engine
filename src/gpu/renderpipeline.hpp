#pragma once

#include "resourcemanager.hpp"
#include "presentmanager.hpp"
#include "mesh.hpp"

#include <vulkan/vulkan.hpp>
#include <eseed/math/mat.hpp>
#include <map>

#define ALIGN_SCLR(type) alignas(sizeof(type))
#define ALIGN_VEC2(type) alignas(2 * sizeof(type))
#define ALIGN_VEC4(type) alignas(4 * sizeof(type))
#define ALIGN_MAT4(type) ALIGN_VEC4(type)
#define ALIGN_STRUCT alignas(16)

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
    ALIGN_VEC4(float) esdm::Vec3<float> position;
    ALIGN_MAT4(float) esdm::Mat4<float> rotation;
    ALIGN_SCLR(float) float aspect;
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