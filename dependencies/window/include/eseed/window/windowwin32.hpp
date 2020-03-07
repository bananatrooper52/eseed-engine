#pragma once

#include "window.hpp"

#ifdef NOMINMAX
#include <window.h>
#else
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif

#include <string>
#include <eseed/math/types.hpp>
#include <eseed/math/vec.hpp>

namespace esd::window {

class WindowWin32 : public Window {
public:
    WindowWin32(esd::math::Vec2<I32> size, std::string title);

    ~WindowWin32();
    
    void poll() override;

    bool isCloseRequested() override;

    void update() override;

    std::vector<const char*> getRequiredInstanceExtensionNames() override;

    vk::SurfaceKHR createSurface(vk::Instance instance) override;

    esd::math::Vec2<I32> getSize() override;

private:
    HINSTANCE hInstance;
    WNDCLASS wc;
    HWND hWnd;
    bool closeRequested = false;

    static LRESULT CALLBACK windowProc(
        HWND hWnd, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam
    );
};

}