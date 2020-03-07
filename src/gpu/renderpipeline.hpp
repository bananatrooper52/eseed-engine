#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <eseed/math/vec.hpp>

class RenderPipeline {
public:
    vk::Device device;
    std::vector<vk::Framebuffer> framebuffers;

    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
    std::vector<vk::CommandBuffer> commandBuffers;

    RenderPipeline(
        vk::Device device, 
        std::vector<vk::ImageView> imageViews,
        vk::SurfaceFormatKHR surfaceFormat,
        vk::CommandPool commandPool,
        esd::math::Vec2<float> size,
        vk::ShaderModule vertShader,
        vk::ShaderModule fragShader
    );

    void addVertexBuffer(vk::Buffer buffer);
    const vk::CommandBuffer& getCommandBuffer(uint32_t i) { return commandBuffers[i]; }

private:
    struct SubpassInfo {
        std::vector<vk::AttachmentReference> attachmentReferences;
        vk::SubpassDescription subpass;
    };

    struct VertexInputStateInfo {
        std::vector<vk::VertexInputBindingDescription> bindings;
        std::vector<vk::VertexInputAttributeDescription> attributes;
        vk::PipelineVertexInputStateCreateInfo vertexInputState;
    };

    struct ViewportStateInfo {
        std::vector<vk::Viewport> viewports;
        std::vector<vk::Rect2D> scissors;
        vk::PipelineViewportStateCreateInfo viewportState;
    };

    struct ColorBlendStateInfo {
        std::vector<vk::PipelineColorBlendAttachmentState> blendStates;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;
    };

    esd::math::Vec2<float> size;

    vk::Buffer vertexBuffer;

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
    static std::vector<SubpassInfo> createSubpasses();
    static std::vector<vk::SubpassDescription> extractSubpasses(
        std::vector<SubpassInfo> infos
    );

    static std::vector<vk::PipelineShaderStageCreateInfo> createStages(
        vk::ShaderModule vertShader,
        vk::ShaderModule fragShader
    );
    static VertexInputStateInfo createVertexInputState();
    static vk::PipelineInputAssemblyStateCreateInfo createInputAssemblyState();
    static ViewportStateInfo createViewportState(esd::math::Vec2<float> size);
    static vk::PipelineRasterizationStateCreateInfo createRasterizationState();
    static vk::PipelineMultisampleStateCreateInfo createMultisampleState();
    static ColorBlendStateInfo createColorBlendState();
    static vk::PipelineLayout createLayout(vk::Device device);
};