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
    RenderContext(std::shared_ptr<esd::window::Window> window = nullptr);
    ~RenderContext();

    void render();
    std::shared_ptr<RenderPipeline> getRenderPipeline();

    std::vector<uint8_t> loadShaderCode(std::string path);
    vk::ShaderModule createShaderModule(const std::vector<uint8_t>& code);

private:
    std::shared_ptr<ResourceManager> rm;
    std::shared_ptr<PresentManager> pm;
    std::shared_ptr<RenderPipeline> pipeline;

    vk::Queue graphicsQueue;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
};