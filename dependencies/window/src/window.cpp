#include <eseed/window/window.hpp>

#include <eseed/window/windowwin32.hpp>

using namespace esd::window;

std::unique_ptr<Window> esd::window::createWindow(
    esd::math::Vec2<I32> size, 
    std::string title
) {
#if defined(_WIN32)
    return std::make_unique<WindowWin32>(size, title);
#elif defined(__linux__)
#error Unix windows are not yet implemented
#elif defined(__APPLE__)
#error Mac windows are not yet implemented
#endif
}