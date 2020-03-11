#include <eseed/window/window.hpp>

#include <eseed/window/windowwin32.hpp>

using namespace esdw;

std::unique_ptr<Window> esdw::createWindow(
    esdm::Vec2<I32> size, 
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