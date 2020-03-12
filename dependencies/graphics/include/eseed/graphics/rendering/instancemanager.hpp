#pragma once

#include <vulkan/vulkan.hpp>

namespace esd::graphics {

// Manages and provides helper functions for a Vulkan instance
class InstanceManager {
public:
	InstanceManager(vk::Instance instance);

	// Get the internal Vulkan instance
	vk::Instance getInstance() const;

	// Return the fastest and most useful physical device capable of supporting 
	// requested surface as well as all requested extensions 
	vk::PhysicalDevice getBestPhysicalDevice(
		std::vector<const char*> extensionNames
	) const;

private:
	vk::Instance instance;

	static int ratePhysicalDevice(
		const vk::PhysicalDevice& physicalDevice,
		const std::vector<const char*>& extensionNames
	);
};

}