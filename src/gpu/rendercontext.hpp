#pragma once

#include "renderpipeline.hpp"
#include "resourcemanager.hpp"
#include "presentmanager.hpp"
#include "meshbuffer.hpp"
#include "mesh.hpp"

#include <eseed/window/window.hpp>
#include <vulkan/vulkan.hpp>
#include <optional>
#include <memory>

class RenderContext {
public:
    RenderContext(std::shared_ptr<esdw::Window> window = nullptr);
    ~RenderContext();

    bool checkFrameAvailable();
    void render();

    std::vector<uint8_t> loadShaderCode(std::string path);
    vk::ShaderModule createShaderModule(const std::vector<uint8_t>& code);

    std::shared_ptr<RenderPipeline> getRenderPipeline();
    std::shared_ptr<ResourceManager> getResourceManager();
    std::shared_ptr<PresentManager> getPresentManager();

private:
    const size_t maxFrameCount = 3;
    size_t currentFrame = 0;

    std::shared_ptr<ResourceManager> rm;
    std::shared_ptr<PresentManager> pm;
    std::shared_ptr<RenderPipeline> pipeline;

    vk::Queue graphicsQueue;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> frameFences;
};