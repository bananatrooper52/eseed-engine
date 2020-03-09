#include "rendercontext.hpp"

#include <fstream>

RenderContext::RenderContext(std::shared_ptr<esd::window::Window> window) {

    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;

    // If a window is provided, include its required extensions
    if (window != nullptr) {
        auto windowExtensions = window->getRequiredInstanceExtensionNames();
        instanceExtensions.insert(
            instanceExtensions.end(), 
            windowExtensions.begin(), 
            windowExtensions.end()
        );
    }

    // If layers are enabled, add them
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");

    std::vector<const char*> deviceExtensions;
    if (window) {
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    rm = std::make_shared<ResourceManager>(
        instanceExtensions,
        instanceLayers,
        deviceExtensions,
        window
    );

    createCommandPool(rm->getDevice(), *rm->getGraphicsQueueFamily());

    imageAvailableSemaphore = rm->getDevice().createSemaphore({});
    renderFinishedSemaphore = rm->getDevice().createSemaphore({});

    /* renderPipeline->registerMeshBuffer(createMeshBuffer(Mesh({
        {{ -1, -1 }, { 1, 0, 0 }},
        {{ 1, -1 }, { 0, 1, 0 }},
        {{ -1, 1 }, { 0, 0, 1 }},
        {{ -1, 1 }, { 0, 0, 1 }},
        {{ 1, -1 }, { 0, 1, 0 }},
        {{ 1, 1 }, { 1, 0, 1 }},
    }))); */
}

void RenderContext::render() {
    /* uint32_t imageIndex = pm->getNextImageIndex(imageAvailableSemaphore);

    std::vector<vk::Semaphore> waitSemaphores = {
        imageAvailableSemaphore
    };
    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eTopOfPipe
    };
    std::vector<vk::Semaphore> signalSemaphores = {
        renderFinishedSemaphore
    };
    auto si = vk::SubmitInfo()
        .setWaitSemaphoreCount((uint32_t)waitSemaphores.size())
        .setPWaitSemaphores(waitSemaphores.data())
        .setPWaitDstStageMask(waitStages.data())
        .setSignalSemaphoreCount((uint32_t)signalSemaphores.size())
        .setPSignalSemaphores(signalSemaphores.data())
        .setCommandBufferCount(1)
        .setPCommandBuffers(&renderPipeline->getCommandBuffer(imageIndex));        

    graphicsQueue.submit(1, &si, nullptr);

    auto pi = vk::PresentInfoKHR()
        .setWaitSemaphoreCount((uint32_t)signalSemaphores.size())
        .setPWaitSemaphores(signalSemaphores.data())
        .setSwapchainCount(1)
        .setPSwapchains(&presentManager->getSwapchain())
        .setPImageIndices(&imageIndex);

    graphicsQueue.presentKHR(pi);

    graphicsQueue.waitIdle(); */
}

std::vector<uint8_t> RenderContext::loadShaderCode(std::string path) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file) throw std::runtime_error("File does not exist");

    std::vector<uint8_t> code;

    file.seekg(0, file.end);
    code.resize(file.tellg());
    file.seekg(0, file.beg);

    file.read((char*)code.data(), code.size());

    return code;
}

vk::ShaderModule RenderContext::createShaderModule(
    const std::vector<uint8_t>& code
) {
    auto shaderModuleCi = vk::ShaderModuleCreateInfo()
        .setCodeSize(code.size())
        .setPCode((uint32_t*)code.data());

    return rm->getDevice().createShaderModule(shaderModuleCi);
}

void RenderContext::createCommandPool(
    vk::Device device,
    uint32_t queueFamily
) {
    commandPool = device.createCommandPool(vk::CommandPoolCreateInfo()
        .setQueueFamilyIndex(queueFamily)
    );
}
