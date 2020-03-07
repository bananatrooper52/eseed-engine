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
        std::vector<vk::Framebuffer> framebuffers,
        vk::SurfaceFormatKHR surfaceFormat,
        vk::CommandPool commandPool,
        esd::math::Vec2<float> size,
        vk::ShaderModule vertShader,
        vk::ShaderModule fragShader
    );

    void addVertexBuffer(vk::Buffer buffer);

private:
    esd::math::Vec2<float> size;

    vk::Buffer vertexBuffer;

    void recordCommandBuffers();

    static std::vector<vk::CommandBuffer> createCommandBuffers(
        vk::Device device,
        std::vector<vk::Framebuffer> framebuffers,
        vk::CommandPool commandPool
    );

    static std::vector<vk::AttachmentDescription> createRenderPassAttachments(
        vk::SurfaceFormatKHR surfaceFormat
    );
    static std::vector<vk::SubpassDescription> createRenderPassSubpasses();

    static std::vector<vk::PipelineShaderStageCreateInfo> createPipelineStages(
        vk::ShaderModule vertShader,
        vk::ShaderModule fragShader
    );
    static vk::PipelineVertexInputStateCreateInfo 
        createPipelineVertexInputState();
    static vk::PipelineInputAssemblyStateCreateInfo
        createPipelineInputAssemblyState();
    static vk::PipelineViewportStateCreateInfo
        createPipelineViewportState(esd::math::Vec2<float> size);
    static vk::PipelineRasterizationStateCreateInfo
        createPipelineRasterizationState();
    static vk::PipelineMultisampleStateCreateInfo
        createPipelineMultisampleState();
    static vk::PipelineColorBlendStateCreateInfo
        createPipelineColorBlendState();
    static vk::PipelineLayout createPipelineLayout(vk::Device device);
};