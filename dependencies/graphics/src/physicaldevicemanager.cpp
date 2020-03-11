#include "eseed/graphics/rendering/physicaldevicemanager.hpp"

#include <eseed/logging/logger.hpp>

using namespace esd::graphics;
using namespace esdl;

PhysicalDeviceManager::PhysicalDeviceManager(
    vk::PhysicalDevice physicalDevice
) 
: physicalDevice(physicalDevice) {}

vk::PhysicalDevice PhysicalDeviceManager::getPhysicalDevice() const {
	return physicalDevice;
}

bool PhysicalDeviceManager::areAllExtensionsAvailable(
    std::vector<const char*> extensionNames
) const {
    auto availableExtensions = 
        physicalDevice.enumerateDeviceExtensionProperties();
    
	std::vector<const char*> missingNames;

	for (auto requiredExtensionName : extensionNames) {
		bool found = false;
		for (auto availableExtension : availableExtensions) {
			if (strcmp(
				availableExtension.extensionName, 
				requiredExtensionName)
			) {
				found = true;
				break;
			}
		}
		if (!found) {
			missingNames.push_back(requiredExtensionName);
		}
	}

    return missingNames.empty();
}

uint32_t PhysicalDeviceManager::findQueueIndex(
	std::function<bool(uint32_t, vk::QueueFamilyProperties)> fn
) const {
	auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
		if (fn(i, queueFamilyProperties[i])) return i;
	}

	throw std::runtime_error(
		mainLogger.error("Could not find queue family")
	);
}

uint32_t PhysicalDeviceManager::findGraphicsQueueIndex() const {
	return findQueueIndex([](uint32_t i, vk::QueueFamilyProperties properties) {
		return bool(properties.queueFlags & vk::QueueFlagBits::eGraphics);
	});
}

uint32_t PhysicalDeviceManager::findPresentQueueIndex(
	vk::SurfaceKHR surface
) const {
	return findQueueIndex([&](
		uint32_t i, vk::QueueFamilyProperties properties
	) {
		return physicalDevice.getSurfaceSupportKHR(i, surface);
	});
}

std::string PhysicalDeviceManager::getName() const {
    return physicalDevice.getProperties().deviceName;
}