#include <eseed/graphics/rendering/instancemanager.hpp>

#include <eseed/graphics/rendering/physicaldevicemanager.hpp>
#include <eseed/logging/logger.hpp>

using namespace esd::graphics;
using namespace esdl;

InstanceManager::InstanceManager(vk::Instance instance) : instance(instance) {}

vk::Instance InstanceManager::getInstance() const { 
	return instance; 
}

vk::PhysicalDevice InstanceManager::getBestPhysicalDevice(
	std::vector<const char*> extensionNames
) const {
	// List of available physical devices
	auto availableList = instance.enumeratePhysicalDevices();

	// Will store best physical device
	vk::PhysicalDevice best;

	// Will store best physical device rating, initialized at -1 to indicate no
	// suitable physical device found
	int bestScore = -1;

	// Loop through each available physical device
	for (auto available : availableList) {
		int score = ratePhysicalDevice(available, extensionNames);
		if (score > bestScore) {
			best = available;
			bestScore = score;
		}
	}

	if (bestScore < 0) {
		throw std::runtime_error(
			mainLogger.error("Could not find a suitable physical device")
		);
	}
	
	return best;
}

int InstanceManager::ratePhysicalDevice(
	const vk::PhysicalDevice& physicalDevice,
	const std::vector<const char*>& extensionNames
) {
	PhysicalDeviceManager pdm(physicalDevice);
	
	auto properties = physicalDevice.getProperties();
	auto features = physicalDevice.getFeatures();
	auto availableExtensions =
		physicalDevice.enumerateDeviceExtensionProperties();

	// The higher the score, the better -- negative value indicates unusable
	// physical device
	int score = 0;

	// A dedicated graphics card is highly favorable
	if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		score += 100;

	// Geometry shader is required
	if (!features.geometryShader) return -1;

	// All extensions are required
	if (!pdm.areAllExtensionsAvailable(extensionNames)) return -1;

	return score;
}