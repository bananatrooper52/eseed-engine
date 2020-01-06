#include <eseed/graphics/window/window.hpp>

#include <eseed/graphics/window/windowwindows.hpp>

std::unique_ptr<eseed::graphics::Window> eseed::graphics::createWindow(eseed::math::Vec2<I32> size, std::string title)
{
#if defined(_WIN32)
    return std::make_unique<eseed::graphics::WindowWindows>(size, title);
#elif defined(__linux__)
#error Unix windows are not yet implemented
#elif defined(__APPLE__)
#error Mac windows are not yet implemented
#endif
}