#include <eseed/graphics/window/window.hpp>

#include <eseed/graphics/window/windowwin32.hpp>

std::unique_ptr<esd::graphics::Window> esd::graphics::createWindow(
    esd::math::Vec2<I32> size, 
    std::string title
) {
#if defined(_WIN32)
    return std::make_unique<esd::graphics::WindowWin32>(size, title);
#elif defined(__linux__)
#error Unix windows are not yet implemented
#elif defined(__APPLE__)
#error Mac windows are not yet implemented
#endif
}