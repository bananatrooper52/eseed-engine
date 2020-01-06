#include <eseed/graphics/window/windowwindows.hpp>

#include <iostream>

eseed::graphics::WindowWindows::WindowWindows(eseed::math::Vec2<I32> size, std::string title)
{
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    const char className[] = "ESeed Graphics Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = (WNDPROC)windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;

    RegisterClass(&wc);

    hWnd = CreateWindowEx(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        size.x,
        size.y,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hWnd == NULL)
    {
        std::cerr << "Failed to create native Windows window" << std::endl;
        throw std::runtime_error("Failed to create native Windows window");
    }

    ShowWindow(hWnd, SW_SHOW);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
}

eseed::graphics::WindowWindows::~WindowWindows()
{
    if (hWnd != NULL)
        DestroyWindow(hWnd);
}

void eseed::graphics::WindowWindows::poll()
{
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

bool eseed::graphics::WindowWindows::isCloseRequested()
{
    return closeRequested;
}

LRESULT CALLBACK eseed::graphics::WindowWindows::windowProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto window = (eseed::graphics::WindowWindows*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
    switch (uMsg)
    {
    case WM_CLOSE:
        window->closeRequested = true;
        return NULL;
    case WM_DESTROY:
        hWnd = NULL;
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}