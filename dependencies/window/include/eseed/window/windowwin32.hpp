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

namespace esdw {

class WindowWin32 : public Window {
public:
    WindowWin32(esdm::Vec2<I32> size, std::string title);

    ~WindowWin32();

    void setKeyDownHandler(std::function<void(KeyCode)> handler) override;
    void setKeyUpHandler(std::function<void(KeyCode)> handler) override;
    
    void poll() override;

    bool isCloseRequested() override;

    void update() override;

    std::vector<const char*> getRequiredInstanceExtensionNames() override;

    vk::SurfaceKHR createSurface(vk::Instance instance) override;

    esdm::Vec2<I32> getSize() override;

private:
    HINSTANCE hInstance;
    WNDCLASS wc;
    HWND hWnd;
    bool closeRequested = false;
    std::function<void(KeyCode)> keyDownHandler;
    std::function<void(KeyCode)> keyUpHandler;

    static LRESULT CALLBACK windowProc(
        HWND hWnd, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam
    );

    static KeyCode findKeyCode(WPARAM wParam);
};

}