#pragma once

#include <eseed/graphics/window/window.hpp>
#include <eseed/logging/logger.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace esd::graphics {

class RenderContext {
public:
	RenderContext(
		std::shared_ptr<esd::graphics::Window> window,
		bool debug,
		esdl::Logger logger);
	void render();

private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		bool hasAllValues() { return graphicsFamily.has_value(); }
	};

	struct SwapchainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	std::shared_ptr<esd::graphics::Window> window;
	bool debug;
	esdl::Logger logger;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;
	std::vector<vk::Framebuffer> swapchainFramebuffers;
	std::vector<vk::CommandBuffer> commandBuffers;
	vk::Format swapchainImageFormat;
	vk::Extent2D swapchainExtent;
	vk::Instance instance;
	vk::PhysicalDevice physicalDevice;
	vk::Device device;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::SurfaceKHR surface;
	vk::SwapchainKHR swapchain;
	vk::RenderPass renderPass;
	vk::Pipeline pipeline;
	vk::CommandPool commandPool;
	vk::Semaphore imageAvailableSemaphore;
	vk::Semaphore renderFinishedSemaphore;

	vk::Instance createInstance();
	std::vector<const char*> getRequiredInstanceExtensions();
	std::vector<const char*> getRequiredInstanceLayers();
	vk::PhysicalDevice selectPhysicalDevice(
		const vk::Instance& instance,
		const vk::SurfaceKHR& surface,
		const std::vector<const char*>& extensions);
	size_t ratePhysicalDevice(
		const vk::PhysicalDevice& physicalDevice,
		const vk::SurfaceKHR& surface,
		const std::vector<const char*>& extensions);
	QueueFamilyIndices findQueueFamilies(
		const vk::PhysicalDevice& physicalDevice,
		const vk::SurfaceKHR& surface);
	vk::Device createDevice(
		const vk::PhysicalDevice& physicalDevice,
		const vk::SurfaceKHR& surface,
		const std::vector<const char*>& extensions);
	SwapchainSupportDetails querySwapchainSupport(
		const vk::PhysicalDevice& physicalDevice,
		const vk::SurfaceKHR& surface);
	vk::SurfaceFormatKHR
	selectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available);
	vk::PresentModeKHR
	selectPresentMode(const std::vector<vk::PresentModeKHR>& available);
	vk::Extent2D
	selectSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	vk::SwapchainKHR createSwapchain(
		const vk::Device& device,
		const vk::SurfaceKHR& surface,
		const SwapchainSupportDetails& swapchainSupport);
	std::vector<vk::ImageView> createImageViews(
		const vk::Device& device,
		const std::vector<vk::Image>& swapchainImages);
	vk::RenderPass createRenderPass(const vk::Device& device);
	vk::Pipeline createGraphicsPipeline(
		const vk::Device& device,
		const std::vector<char>& vertCode,
		const std::vector<char>& fragCode);
	std::vector<char> loadShaderBinary(const std::string& path);
	vk::ShaderModule
	createShaderModule(const vk::Device& device, const std::vector<char>& code);
	void createSwapchainFramebuffers();
	void createCommandPool();
	void allocCommandBuffers();
	void createSemaphores();
};

} // namespace esd::graphics