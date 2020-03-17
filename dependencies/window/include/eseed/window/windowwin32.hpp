#pragma once

#include "window.hpp"

#ifdef NOMINMAX
#include <window.h>
#else
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif

#include <eseed/math/types.hpp>
#include <eseed/math/vec.hpp>
#include <string>

namespace esdw {

class WindowWin32 : public Window {
public:
    WindowWin32(esdm::Vec2<I32> size, std::string title);

    ~WindowWin32();

    void setKeyDownHandler(std::function<void(KeyDownEvent)> handler) override;
    void setKeyUpHandler(std::function<void(KeyUpEvent)> handler) override;
    void setMouseMoveHandler(std::function<void(MouseMoveEvent)> handler) override;

    bool getKey(KeyCode keyCode) override;
    MousePos getMouseScreenPos() override;
    MousePos getMousePos() override;
    void setMouseScreenPos(MousePos screenPos) override;
    void setMousePos(MousePos pos) override;
    
    void poll() override;

    bool isCloseRequested() override;

    void update() override;

    esdm::Vec2<I32> getSize() override;

    std::vector<const char*> getRequiredInstanceExtensionNames() override;

    vk::SurfaceKHR createSurface(vk::Instance instance) override;

private:
    HINSTANCE hInstance;
    WNDCLASS wc;
    HWND hWnd;
    bool closeRequested = false;
    std::function<void(KeyDownEvent)> keyDownHandler;
    std::function<void(KeyUpEvent)> keyUpHandler;
    std::function<void(MouseMoveEvent)> mouseMoveHandler;

    static LRESULT CALLBACK windowProc(
        HWND hWnd, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam
    );
};

}