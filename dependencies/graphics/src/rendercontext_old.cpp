#define NOMINMAX

#include <eseed/graphics/rendering/rendercontext_old.hpp>

#include <eseed/math/vec.hpp>

#include <algorithm>
#include <cstdint>
#include <fstream>

using namespace esd::graphics;
using namespace esdl;
using namespace esdm;

RenderContext::RenderContext(
	std::shared_ptr<Window> window, bool debug, Logger logger)
	: window(window), debug(debug), logger(logger)
{
	instance = createInstance();
	surface = window->createSurface(instance);

	std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	physicalDevice = selectPhysicalDevice(instance, surface, extensions);
	logger.info(
		"Selected physical device: {}",
		physicalDevice.getProperties().deviceName);
///
	device = createDevice(physicalDevice, surface, extensions);
	logger.info("Device created");
///
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
	logger.info("Located queue families");
	logger.debug("Graphics queue family: {}", indices.graphicsFamily.value());

	graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
	presentQueue = device.getQueue(indices.graphicsFamily.value(), 0);
	logger.info("Created queues");
///
	swapchain = createSwapchain(
		device, surface, querySwapchainSupport(physicalDevice, surface));
	logger.info("Created swapchain");
///
	swapchainImages = device.getSwapchainImagesKHR(swapchain);
	swapchainImageViews = createImageViews(device, swapchainImages);
	logger.info("Created image views");
	logger.debug("Image view count: {}", swapchainImageViews.size());

	std::vector<char> vertCode =
		loadShaderBinary("resources/shaders/test/test_vert.spv");
	std::vector<char> fragCode =
		loadShaderBinary("resources/shaders/test/test_frag.spv");
	logger.info("Loaded shader sources");

	renderPass = createRenderPass(device);
	logger.info("Created render pass");

	pipeline = createGraphicsPipeline(device, vertCode, fragCode);
	logger.info("Created pipeline");

	createSwapchainFramebuffers();
	logger.info("Created swapchain framebuffers");

	createCommandPool();
	logger.info("Created command pool");

	allocCommandBuffers();
	logger.info("Allocated command buffers");
	logger.debug("Command buffer count: {}", commandBuffers.size());

	createSemaphores();
	logger.info("Created semaphores");

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		auto commandBufferBi = vk::CommandBufferBeginInfo();

		commandBuffers[i].begin(commandBufferBi);

		auto clearColor = vk::ClearValue().setColor(
			vk::ClearColorValue(std::array<float, 4> {0.f, 0.f, 0.f, 1.f}));

		auto renderPassBi =
			vk::RenderPassBeginInfo()
				.setRenderPass(renderPass)
				.setFramebuffer(swapchainFramebuffers[i])
				.setRenderArea(vk::Rect2D().setExtent(swapchainExtent))
				.setClearValueCount(1)
				.setPClearValues(&clearColor);

		commandBuffers[i].beginRenderPass(
			&renderPassBi, vk::SubpassContents::eInline);

		commandBuffers[i].bindPipeline(
			vk::PipelineBindPoint::eGraphics, pipeline);
		commandBuffers[i].draw(3, 1, 0, 0);

		commandBuffers[i].endRenderPass();

		commandBuffers[i].end();
	}
}

void RenderContext::render()
{
	uint32_t imageIndex;
	imageIndex =
		device
			.acquireNextImageKHR(
				swapchain, UINT64_MAX, imageAvailableSemaphore, nullptr)
			.value;

	std::vector<vk::Semaphore> waitSemaphores = {imageAvailableSemaphore};
	std::vector<vk::PipelineStageFlags> waitStages = {
		vk::PipelineStageFlagBits::eColorAttachmentOutput};

	auto si = vk::SubmitInfo();
	si.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
	si.pWaitSemaphores = waitSemaphores.data();
	si.pWaitDstStageMask = waitStages.data();
	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffers[imageIndex];

	std::vector<vk::Semaphore> signalSemaphores = {renderFinishedSemaphore};
	si.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
	si.pSignalSemaphores = signalSemaphores.data();

	graphicsQueue.submit(1, &si, nullptr);

	std::vector<vk::SwapchainKHR> swapchains = {swapchain};

	auto pi = vk::PresentInfoKHR()
				  .setWaitSemaphoreCount((uint32_t)signalSemaphores.size())
				  .setPWaitSemaphores(signalSemaphores.data())
				  .setSwapchainCount((uint32_t)swapchains.size())
				  .setPSwapchains(swapchains.data())
				  .setPImageIndices(&imageIndex);

	presentQueue.presentKHR(pi);
}

vk::Instance RenderContext::createInstance()
{
	vk::ApplicationInfo applicationInfo = {};
	applicationInfo.pApplicationName = "ESeed Engine Game";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "ESeed Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_1;

	auto requiredInstanceLayers = getRequiredInstanceLayers();
	auto requiredInstanceExtensions = getRequiredInstanceExtensions();

	vk::InstanceCreateInfo ci = {};
	ci.pApplicationInfo = &applicationInfo;
	ci.ppEnabledExtensionNames = requiredInstanceExtensions.data();
	ci.enabledExtensionCount = (uint32_t)requiredInstanceExtensions.size();
	ci.ppEnabledLayerNames = requiredInstanceLayers.data();
	ci.enabledLayerCount = (uint32_t)requiredInstanceLayers.size();

	return vk::createInstance(ci);
}

std::vector<const char*> RenderContext::getRequiredInstanceExtensions()
{
	return window->getRequiredInstanceExtensionNames();
}

std::vector<const char*> RenderContext::getRequiredInstanceLayers()
{
	if (debug)
	{
		logger.info(
			"Render context debug is enabled, enabling validation layers");
		return {"VK_LAYER_KHRONOS_validation"};
	}
	else
	{
		logger.info(
			"Render context debug not enabled, validation layers will not be utilized");
		return {};
	}
}

vk::PhysicalDevice RenderContext::selectPhysicalDevice(
	const vk::Instance& instance,
	const vk::SurfaceKHR& surface,
	const std::vector<const char*>& extensions)
{
	std::vector<vk::PhysicalDevice> physicalDevices =
		instance.enumeratePhysicalDevices();
	std::vector<std::pair<vk::PhysicalDevice, size_t>> physicalDeviceRatings;

	for (const auto& physicalDevice : physicalDevices)
	{
		if (size_t rating =
				ratePhysicalDevice(physicalDevice, surface, extensions) > 0)
		{
			physicalDeviceRatings.push_back(
				std::make_pair(physicalDevice, rating));
		}
	}

	std::sort(
		physicalDeviceRatings.begin(),
		physicalDeviceRatings.end(),
		[](const auto& a, const auto& b) { return a.second > b.second; });

	if (physicalDevices.empty())
	{
		std::string errString = "Could not find a suitable physical device!";
		logger.error(errString);
		throw std::runtime_error(errString);
	}

	vk::PhysicalDevice physicalDevice = physicalDevices[0];

	return physicalDevice;
}

size_t RenderContext::ratePhysicalDevice(
	const vk::PhysicalDevice& physicalDevice,
	const vk::SurfaceKHR& surface,
	const std::vector<const char*>& extensions)
{
	size_t rating = 1;

	auto properties = physicalDevice.getProperties();
	auto features = physicalDevice.getFeatures();

	if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		rating += 100;

	if (!features.geometryShader) rating = 0;

	auto availableExtensions =
		physicalDevice.enumerateDeviceExtensionProperties();

	for (const auto& it : extensions)
	{
		if (std::find_if(
				availableExtensions.begin(),
				availableExtensions.end(),
				[&](const auto& extension) {
					return extension.extensionName == it;
				}) == availableExtensions.end())
			rating = 0;
	}

	SwapchainSupportDetails swapchainSupport =
		querySwapchainSupport(physicalDevice, surface);
	if (swapchainSupport.formats.empty() ||
		swapchainSupport.presentModes.empty())
		rating = 0;

	return rating;
}

RenderContext::QueueFamilyIndices RenderContext::findQueueFamilies(
	const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
	QueueFamilyIndices indices;

	auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags &
				vk::QueueFlagBits::eGraphics &&
			physicalDevice.getSurfaceSupportKHR(i, surface))
			indices.graphicsFamily = i;

		if (indices.hasAllValues()) break;
	}

	if (!indices.hasAllValues())
	{
		std::string errString =
			"Some physical device queue families could not be found";
		logger.error(errString);
		throw std::runtime_error(errString);
	}

	return indices;
}

vk::Device RenderContext::createDevice(
	const vk::PhysicalDevice& physicalDevice,
	const vk::SurfaceKHR& surface,
	const std::vector<const char*>& extensions)
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	float priority = 1;

	vk::DeviceQueueCreateInfo queueCi;
	queueCi.queueFamilyIndex = indices.graphicsFamily.value();
	queueCi.queueCount = 1;
	queueCi.pQueuePriorities = &priority;

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo ci;
	ci.pQueueCreateInfos = &queueCi;
	ci.queueCreateInfoCount = 1;
	ci.pEnabledFeatures = &features;
	ci.enabledExtensionCount = (uint32_t)extensions.size();
	ci.ppEnabledExtensionNames = extensions.data();
	ci.enabledLayerCount = 0;

	auto device = physicalDevice.createDevice(ci);

	return device;
}

RenderContext::SwapchainSupportDetails RenderContext::querySwapchainSupport(
	const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
	SwapchainSupportDetails details;

	details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
	details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	return details;
}

vk::SurfaceFormatKHR RenderContext::selectSurfaceFormat(
	const std::vector<vk::SurfaceFormatKHR>& available)
{
	for (const auto& it : available)
	{
		if (it.format == vk::Format::eB8G8R8A8Unorm &&
			it.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return it;
	}

	std::string errString = "Could not find a suitable surface format";
	logger.error(errString);
	throw std::runtime_error(errString);
}

vk::PresentModeKHR RenderContext::selectPresentMode(
	const std::vector<vk::PresentModeKHR>& available)
{
	for (const auto& it : available)
	{
		if (it == vk::PresentModeKHR::eMailbox) return it;
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D RenderContext::selectSwapchainExtent(
	const vk::SurfaceCapabilitiesKHR& capabilities)
{
	Vec2<uint32_t> windowSize(window->getSize());
	windowSize.x = std::max(
		capabilities.minImageExtent.width,
		std::min(capabilities.maxImageExtent.width, windowSize.x));
	windowSize.y = std::max(
		capabilities.minImageExtent.height,
		std::min(capabilities.maxImageExtent.height, windowSize.y));
	return {windowSize.x, windowSize.y};
}

vk::SwapchainKHR RenderContext::createSwapchain(
	const vk::Device& device,
	const vk::SurfaceKHR& surface,
	const SwapchainSupportDetails& swapchainSupport)
{
	auto surfaceFormat = selectSurfaceFormat(swapchainSupport.formats);
	auto presentMode = selectPresentMode(swapchainSupport.presentModes);
	auto extent = selectSwapchainExtent(swapchainSupport.capabilities);

	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

	if (swapchainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapchainSupport.capabilities.maxImageCount)
		imageCount = swapchainSupport.capabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR ci;
	ci.surface = surface;
	ci.minImageCount = imageCount;
	ci.imageFormat = surfaceFormat.format;
	ci.imageColorSpace = surfaceFormat.colorSpace;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	ci.imageSharingMode = vk::SharingMode::eExclusive;
	ci.queueFamilyIndexCount = 0;
	ci.pQueueFamilyIndices = nullptr;
	ci.preTransform = swapchainSupport.capabilities.currentTransform;
	ci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	ci.presentMode = presentMode;
	ci.clipped = true;
	ci.oldSwapchain = nullptr;

	return device.createSwapchainKHR(ci);
}

std::vector<vk::ImageView> RenderContext::createImageViews(
	const vk::Device& device, const std::vector<vk::Image>& swapchainImages)
{
	std::vector<vk::ImageView> swapchainImageViews(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		vk::ImageViewCreateInfo ci;
		ci.image = swapchainImages[i];
		ci.viewType = vk::ImageViewType::e2D;
		ci.format = swapchainImageFormat;
		ci.components = {vk::ComponentSwizzle::eIdentity,
						 vk::ComponentSwizzle::eIdentity,
						 vk::ComponentSwizzle::eIdentity,
						 vk::ComponentSwizzle::eIdentity};
		ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = 1;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = 1;

		swapchainImageViews[i] = device.createImageView(ci);
	}

	return swapchainImageViews;
}

vk::RenderPass RenderContext::createRenderPass(const vk::Device& device)
{
	auto attachmentDesc =
		vk::AttachmentDescription()
			.setFormat(swapchainImageFormat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	auto attachmentRef = vk::AttachmentReference().setAttachment(0).setLayout(
		vk::ImageLayout::eColorAttachmentOptimal);

	auto subpassDesc =
		vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&attachmentRef);

	auto ci = vk::RenderPassCreateInfo()
				  .setAttachmentCount(1)
				  .setPAttachments(&attachmentDesc)
				  .setSubpassCount(1)
				  .setPSubpasses(&subpassDesc);

	return device.createRenderPass(ci);
}

vk::Pipeline RenderContext::createGraphicsPipeline(
	const vk::Device& device,
	const std::vector<char>& vertCode,
	const std::vector<char>& fragCode)
{
	auto vertModule = createShaderModule(device, vertCode);
	auto fragModule = createShaderModule(device, fragCode);

	vk::PipelineShaderStageCreateInfo vertStageCi;
	vertStageCi.stage = vk::ShaderStageFlagBits::eVertex;
	vertStageCi.module = vertModule;
	vertStageCi.pName = "main";

	vk::PipelineShaderStageCreateInfo fragStageCi;
	fragStageCi.stage = vk::ShaderStageFlagBits::eFragment;
	fragStageCi.module = fragModule;
	fragStageCi.pName = "main";

	std::vector<vk::PipelineShaderStageCreateInfo> stageCis = {vertStageCi,
															   fragStageCi};

	vk::PipelineVertexInputStateCreateInfo vertexInputCi;
	vertexInputCi.vertexBindingDescriptionCount = 0;
	vertexInputCi.pVertexBindingDescriptions = nullptr;
	vertexInputCi.vertexAttributeDescriptionCount = 0;
	vertexInputCi.pVertexAttributeDescriptions = nullptr;

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCi;
	inputAssemblyCi.topology = vk::PrimitiveTopology::eTriangleList;

	vk::Viewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = (float)swapchainExtent.width;
	viewport.height = (float)swapchainExtent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vk::Rect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent = swapchainExtent;

	vk::PipelineViewportStateCreateInfo viewportStateCi = {};
	viewportStateCi.viewportCount = 1;
	viewportStateCi.pViewports = &viewport;
	viewportStateCi.scissorCount = 1;
	viewportStateCi.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizerCi;
	rasterizerCi.depthClampEnable = false;
	rasterizerCi.rasterizerDiscardEnable = false;
	rasterizerCi.polygonMode = vk::PolygonMode::eFill;
	rasterizerCi.lineWidth = 1.f;
	rasterizerCi.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizerCi.depthBiasEnable = false;

	auto multisampleCi =
		vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	auto colorBlendAttachment =
		vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(false);

	auto colorBlendCi = vk::PipelineColorBlendStateCreateInfo()
							.setLogicOpEnable(false)
							.setAttachmentCount(1)
							.setPAttachments(&colorBlendAttachment);

	auto pipelineLayoutCi = vk::PipelineLayoutCreateInfo();
	auto pipelineLayout = device.createPipelineLayout(pipelineLayoutCi);

	auto ci = vk::GraphicsPipelineCreateInfo()
				  .setStageCount(2)
				  .setPStages(stageCis.data())
				  .setPVertexInputState(&vertexInputCi)
				  .setPViewportState(&viewportStateCi)
				  .setPRasterizationState(&rasterizerCi)
				  .setPMultisampleState(&multisampleCi)
				  .setPColorBlendState(&colorBlendCi)
				  .setPInputAssemblyState(&inputAssemblyCi)
				  .setLayout(pipelineLayout)
				  .setRenderPass(renderPass)
				  .setSubpass(0);

	return device.createGraphicsPipeline(nullptr, ci);
}

std::vector<char> RenderContext::loadShaderBinary(const std::string& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::string errString =
			format("Could not load shader binary: \"{}\"", path);
		logger.error(errString);
		throw std::runtime_error(errString);
	}

	size_t fileSize = (size_t)file.tellg();

	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

vk::ShaderModule RenderContext::createShaderModule(
	const vk::Device& device, const std::vector<char>& code)
{
	vk::ShaderModuleCreateInfo ci;
	ci.codeSize = code.size();
	ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

	return device.createShaderModule(ci);
}

void RenderContext::createSwapchainFramebuffers()
{
	swapchainFramebuffers.resize(swapchainImageViews.size());
	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		std::vector<vk::ImageView> attachments = {swapchainImageViews[i]};

		auto ci = vk::FramebufferCreateInfo()
					  .setRenderPass(renderPass)
					  .setAttachmentCount(1)
					  .setPAttachments(attachments.data())
					  .setWidth(swapchainExtent.width)
					  .setHeight(swapchainExtent.height)
					  .setLayers(1);

		swapchainFramebuffers[i] = device.createFramebuffer(ci);
	}
}

void RenderContext::createCommandPool()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	auto ci = vk::CommandPoolCreateInfo().setQueueFamilyIndex(
		indices.graphicsFamily.value());

	commandPool = device.createCommandPool(ci);
}

void RenderContext::allocCommandBuffers()
{
	auto ai =
		vk::CommandBufferAllocateInfo()
			.setCommandPool(commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount((uint32_t)swapchainFramebuffers.size());

	commandBuffers = device.allocateCommandBuffers(ai);
}

void RenderContext::createSemaphores()
{
	auto ci = vk::SemaphoreCreateInfo();
	imageAvailableSemaphore = device.createSemaphore(ci);
	renderFinishedSemaphore = device.createSemaphore(ci);
}