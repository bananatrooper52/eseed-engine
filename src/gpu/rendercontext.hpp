#pragma once

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

    void render();
    std::vector<uint8_t> loadShaderCode(std::string path);
    std::shared_ptr<MeshBuffer> createMeshBuffer(const Mesh& mesh);

private:
    std::shared_ptr<ResourceManager> rm;
    std::shared_ptr<PresentManager> pm;

    // Commands
    vk::CommandPool commandPool;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;

    vk::ShaderModule createShaderModule(
        const std::vector<uint8_t>& code
    );

    void createCommandPool(
        vk::Device device,
        uint32_t queueFamily
    );
};