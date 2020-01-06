#pragma once

#include <string>
#include <Windows.h>
#include <eseed/math/types.hpp>
#include <eseed/math/vec.hpp>
#include "window.hpp"

namespace eseed::graphics
{

class WindowWindows : public Window
{
public:
    WindowWindows(eseed::math::Vec2<I32> size, std::string title);
    ~WindowWindows();
    void poll() override;
    bool isCloseRequested() override;

private:
    HINSTANCE hInstance;
    WNDCLASS wc;
    HWND hWnd;
    bool closeRequested = false;

    static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

}