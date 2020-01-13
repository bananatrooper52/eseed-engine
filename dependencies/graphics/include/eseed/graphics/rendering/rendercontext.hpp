#pragma once

#include <eseed/graphics/rendering/devicemanager.hpp>
#include <eseed/graphics/rendering/instancemanager.hpp>
#include <eseed/graphics/rendering/physicaldevicemanager.hpp>
#include <eseed/graphics/rendering/surfacemanager.hpp>
#include <eseed/graphics/rendering/swapchainmanager.hpp>
#include <eseed/graphics/window/window.hpp>
#include <vulkan/vulkan.hpp>

namespace esd::graphics {

class RenderContext {
public:
	// Construct an uninitialized context
	RenderContext(
		std::shared_ptr<Window> window,
		bool debugEnabled = false
	);

	// Initialize the context
	void init();

	// Render
	void render();

private:
	bool debugEnabled = false;
	std::shared_ptr<Window> window;
	std::unique_ptr<InstanceManager> instanceManager;
	std::unique_ptr<PhysicalDeviceManager> physicalDeviceManager;
	std::unique_ptr<DeviceManager> deviceManager;
	std::unique_ptr<SurfaceManager> surfaceManager;
	std::unique_ptr<SwapchainManager> swapchainManager;
	vk::Queue graphicsQueue, graphicsQueue;

	void initInstanceManager();

	void initSurfaceManager();

	void initPhysicalDeviceManager();

	void initDeviceManager();

	void initQueues();

	void initSwapchainManager();

	std::vector<const char*> getRequiredInstanceExtensionNames();

	std::vector<const char*> getRequiredInstanceLayerNames();

	std::vector<const char*> getRequiredDeviceExtensionNames();
};

} 