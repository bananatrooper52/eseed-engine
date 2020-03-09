#include "presentmanager.hpp"

PresentManager::PresentManager(
    std::shared_ptr<ResourceManager> rm
) : rm(rm) {
    findSwapchainFormat();
    createSwapchain();
    createSwapchainImageViews();
}

uint32_t PresentManager::getNextImageIndex(vk::Semaphore semaphore) {
    return rm->getDevice().acquireNextImageKHR(
        swapchain, 
        UINT64_MAX, 
        semaphore, 
        nullptr
    ).value;
}

vk::ImageView PresentManager::getImageView(uint32_t index) {
    return swapchainImageViews.at((size_t)index);
}

vk::SwapchainKHR PresentManager::getSwapchain() {
    return swapchain;
}

void PresentManager::findSwapchainFormat() {
    vk::SurfaceFormatKHR swapchainFormat;
    for (auto format : *rm->getSurfaceFormats()) {
        // TODO: actually search lol
        swapchainFormat = format;
        break;
    }
}

void PresentManager::createSwapchain() {
    auto surfaceCapabilities = *rm->getSurfaceCapabilities();

    swapchain = rm->getDevice().createSwapchainKHR(vk::SwapchainCreateInfoKHR()
        .setSurface(*rm->getSurface())
        .setMinImageCount(surfaceCapabilities.minImageCount)
        .setImageExtent(surfaceCapabilities.maxImageExtent)
        .setImageFormat(swapchainFormat.format)
        .setPreTransform(surfaceCapabilities.currentTransform)
        .setPresentMode(vk::PresentModeKHR::eFifo)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageArrayLayers(1)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setClipped(true)
    );
}

void PresentManager::createSwapchainImageViews() {
    auto images = rm->getDevice().getSwapchainImagesKHR(swapchain);

    swapchainImageViews.resize(images.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        auto imageViewCi = vk::ImageViewCreateInfo()
            .setImage(images[i])
            .setFormat(swapchainFormat.format)
            .setComponents(vk::ComponentMapping {}) // Empty cnstr, all identity
            .setViewType(vk::ImageViewType::e2D)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
            );
        swapchainImageViews[i] = 
            rm->getDevice().createImageView(imageViewCi);
    }
}