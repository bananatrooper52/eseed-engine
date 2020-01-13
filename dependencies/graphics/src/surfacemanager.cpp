#include <eseed/graphics/rendering/surfacemanager.hpp>

using namespace esd::graphics;

SurfaceManager::SurfaceManager(vk::SurfaceKHR surface) : surface(surface) {}

vk::SurfaceKHR SurfaceManager::getSurface() const {
    return surface;
}