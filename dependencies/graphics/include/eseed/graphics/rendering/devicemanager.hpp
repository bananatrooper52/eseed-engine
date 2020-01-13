#pragma once

#include <vulkan/vulkan.hpp>

namespace esd::graphics {

class DeviceManager {
public:
	DeviceManager(vk::Device device);

	vk::Device getDevice() const;

private:
	vk::Device device;
};

}