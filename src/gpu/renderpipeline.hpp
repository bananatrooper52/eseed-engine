#pragma once

#include "mesh.hpp"
#include "meshbuffer.hpp"

#include <vulkan/vulkan.hpp>
#include <vector>
#include <eseed/math/vec.hpp>

class RenderPipeline {
public:
    RenderPipeline(
        vk::Device device, 
        std::vector<vk::ImageView> imageViews,
        vk::SurfaceFormatKHR surfaceFormat,
        vk::CommandPool commandPool,
        esd::math::Vec2<float> size,
        vk::ShaderModule vertShader,
        vk::ShaderModule fragShader
    );

    ~RenderPipeline();

    size_t registerMeshBuffer(std::shared_ptr<MeshBuffer> meshBuffer);
    void unregisterMeshBuffer(size_t id);
    const vk::CommandBuffer& getCommandBuffer(uint32_t i) { return commandBuffers[i]; }

private:
    struct SubpassContainer {
        std::vector<vk::AttachmentReference> attachmentReferences;
        vk::SubpassDescription subpass;

        SubpassContainer(
            std::vector<vk::AttachmentReference> _attachmentReferences
        );
    };

    struct VertexInputStateContainer {
        std::vector<vk::VertexInputBindingDescription> bindings;
        std::vector<vk::VertexInputAttributeDescription> attributes;
        vk::PipelineVertexInputStateCreateInfo vertexInputState;

        VertexInputStateContainer(
            std::vector<vk::VertexInputBindingDescription> _bindings,
            std::vector<vk::VertexInputAttributeDescription> _attributes
        );
    };

    struct ViewportStateContainer {
        std::vector<vk::Viewport> viewports;
        std::vector<vk::Rect2D> scissors;
        vk::PipelineViewportStateCreateInfo viewportState;

        ViewportStateContainer(
            std::vector<vk::Viewport> _viewports,
            std::vector<vk::Rect2D> _scissors
        );
    };

    struct ColorBlendStateContainer {
        std::vector<vk::PipelineColorBlendAttachmentState> blendStates;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;

        ColorBlendStateContainer(
            std::vector<vk::PipelineColorBlendAttachmentState> _blendStates
        );
    };

    struct LayoutContainer {
        vk::Device device;
        
        vk::PipelineLayout layout;

        LayoutContainer(
            vk::Device device,
            std::vector<std::vector<vk::DescriptorSetLayoutBinding>> _setLayouts
        );

        ~LayoutContainer();
    };

    esd::math::Vec2<float> size;

    vk::Device device;
    std::vector<vk::Framebuffer> framebuffers;
    vk::CommandPool commandPool;
    std::vector<std::shared_ptr<MeshBuffer>> meshBuffers;

    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
    std::vector<vk::CommandBuffer> commandBuffers;

    void recordCommandBuffers();

    static std::vector<vk::CommandBuffer> createCommandBuffers(
        vk::Device device,
        vk::CommandPool commandPool,
        uint32_t count
    );

    static std::vector<vk::Framebuffer> createFramebuffers(
        vk::Device device,
        vk::RenderPass renderPass,
        std::vector<vk::ImageView> imageViews,
        esd::math::Vec2<float> size
    );

    static std::vector<vk::AttachmentDescription> createAttachments(
        vk::SurfaceFormatKHR surfaceFormat
    );
    static std::vector<SubpassContainer> createSubpasses();
    static std::vector<vk::SubpassDescription> extractSubpasses(
        std::vector<SubpassContainer> infos
    );

    static std::vector<vk::PipelineShaderStageCreateInfo> createStages(
        vk::ShaderModule vertShader,
        vk::ShaderModule fragShader
    );
    static VertexInputStateContainer createVertexInputState();
    static vk::PipelineInputAssemblyStateCreateInfo createInputAssemblyState();
    static ViewportStateContainer createViewportState(esd::math::Vec2<float> size);
    static vk::PipelineRasterizationStateCreateInfo createRasterizationState();
    static vk::PipelineMultisampleStateCreateInfo createMultisampleState();
    static ColorBlendStateContainer createColorBlendState();
    static LayoutContainer createLayout(vk::Device device);
};