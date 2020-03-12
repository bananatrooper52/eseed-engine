#define VK_USE_PLATFORM_WIN32_KHR

#include <eseed/graphics/window/windowwin32.hpp>

#include <eseed/logging/logger.hpp>

using namespace esd::graphics;
using namespace esdl;
using namespace esdm;

WindowWin32::WindowWin32(esdm::Vec2<I32> size, std::string title) {
    mainLogger.debug("Creating window \"{}\"", title);
    
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    const char className[] = "ESeed Graphics Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = (WNDPROC)windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

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
        NULL
    );

    if (hWnd == NULL) {
        throw std::runtime_error(
            mainLogger.error("Failed to create native Windows window")
        );
    }

    ShowWindow(hWnd, SW_SHOW);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
}

WindowWin32::~WindowWin32() {
    if (hWnd != NULL) DestroyWindow(hWnd);
}

void WindowWin32::poll() {
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

bool WindowWin32::isCloseRequested() {
    return closeRequested;
}

LRESULT CALLBACK WindowWin32::windowProc(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
) {
    auto window = (WindowWin32 *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

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

void WindowWin32::update() {
    RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW);
}

std::vector<const char*> WindowWin32::getRequiredInstanceExtensionNames() {
    return {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
}

vk::SurfaceKHR WindowWin32::createSurface(vk::Instance instance) {
    vk::Win32SurfaceCreateInfoKHR ci;
    ci.hwnd = hWnd;
    ci.hinstance = hInstance;

    auto surface = instance.createWin32SurfaceKHR(ci);

    mainLogger.debug("Win32 surface created");

    return surface;
}

Vec2<I32> WindowWin32::getSize() {
    RECT rect;
    if (GetWindowRect(hWnd, &rect)) {
        return { rect.right - rect.left, rect.bottom - rect.top };
    } else { 
        throw std::runtime_error(
            mainLogger.error("Could not get win32 window size")
        );
    }
}