#include <eseed/graphics/rendering/devicemanager.hpp>

using namespace esd::graphics;

DeviceManager::DeviceManager(vk::Device device) : device(device) {}

vk::Device DeviceManager::getDevice() const {
    return device;
}