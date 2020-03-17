#define VK_USE_PLATFORM_WIN32_KHR

#include <eseed/window/windowwin32.hpp>

#include <eseed/logging/logger.hpp>
#include <windowsx.h>
#include <set>

using namespace esdw;
using namespace esdl;
using namespace esdm;

const std::set<std::pair<KeyCode, int>> keyCodeMappings = {
    { KeyBackspace, VK_BACK },
    { KeyTab, VK_TAB },
    { KeyClear, VK_CLEAR },
    { KeyEnter, VK_RETURN },
    { KeyShift, VK_SHIFT },
    { KeyCtrl, VK_CONTROL },
    { KeyAlt, VK_MENU },
    { KeyPause, VK_PAUSE },
    { KeyCapsLock, VK_CAPITAL },
    { KeyEsc, VK_ESCAPE },
    { KeySpace, VK_SPACE },
    { KeyPgUp, VK_PRIOR },
    { KeyPgDn, VK_NEXT },
    { KeyEnd, VK_END },
    { KeyHome, VK_HOME },
    { KeyArrowLeft, VK_LEFT },
    { KeyArrowUp, VK_UP },
    { KeyArrowRight, VK_RIGHT },
    { KeyArrowDown, VK_DOWN },
    { KeySelect, VK_SELECT },
    { KeyPrntScrn, VK_SNAPSHOT },
    { KeyIns, VK_INSERT },
    { KeyDel, VK_DELETE },
    { Key0, 0x30 },
    { Key1, 0x31 },
    { Key2, 0x32 },
    { Key3, 0x33 },
    { Key4, 0x34 },
    { Key5, 0x35 },
    { Key6, 0x36 },
    { Key7, 0x37 },
    { Key8, 0x38 },
    { Key9, 0x39 },
    { KeyA, 0x41 },
    { KeyB, 0x42 },
    { KeyC, 0x43 },
    { KeyD, 0x44 },
    { KeyE, 0x45 },
    { KeyF, 0x46 },
    { KeyG, 0x47 },
    { KeyH, 0x48 },
    { KeyI, 0x49 },
    { KeyJ, 0x4a },
    { KeyK, 0x4b },
    { KeyL, 0x4c },
    { KeyM, 0x4d },
    { KeyN, 0x4e },
    { KeyO, 0x4f },
    { KeyP, 0x50 },
    { KeyQ, 0x51 },
    { KeyR, 0x52 },
    { KeyS, 0x53 },
    { KeyT, 0x54 },
    { KeyU, 0x55 },
    { KeyV, 0x56 },
    { KeyW, 0x57 },
    { KeyX, 0x58 },
    { KeyY, 0x59 },
    { KeyZ, 0x5a },
    { KeyMetaL, VK_LWIN },
    { KeyMetaR, VK_RWIN },
    { KeyNumpad0, VK_NUMPAD0 },
    { KeyNumpad1, VK_NUMPAD1 },
    { KeyNumpad2, VK_NUMPAD2 },
    { KeyNumpad3, VK_NUMPAD3 },
    { KeyNumpad4, VK_NUMPAD4 },
    { KeyNumpad5, VK_NUMPAD5 },
    { KeyNumpad6, VK_NUMPAD6 },
    { KeyNumpad7, VK_NUMPAD7 },
    { KeyNumpad8, VK_NUMPAD8 },
    { KeyNumpad9, VK_NUMPAD9 },
    { KeyMul, VK_MULTIPLY },
    { KeyAdd, VK_ADD },
    { KeySub, VK_SUBTRACT },
    { KeyDecimal, VK_DECIMAL },
    { KeyDiv, VK_DIVIDE },
    { KeyF1, VK_F1 },
    { KeyF2, VK_F2 },
    { KeyF3, VK_F3 },
    { KeyF4, VK_F4 },
    { KeyF5, VK_F5 },
    { KeyF6, VK_F6 },
    { KeyF7, VK_F7 },
    { KeyF8, VK_F8 },
    { KeyF9, VK_F9 },
    { KeyF10, VK_F10 },
    { KeyF11, VK_F11 },
    { KeyF12, VK_F12 },
    { KeyNumLock, VK_NUMLOCK },
    { KeyScrollLock, VK_SCROLL },
    { KeySemicolon, VK_OEM_1 },
    { KeyEqual, VK_OEM_PLUS },
    { KeyComma, VK_OEM_COMMA },
    { KeyDash, VK_OEM_MINUS },
    { KeyPeriod, VK_OEM_PERIOD },
    { KeySlash, VK_OEM_2 },
    { KeyGrave, VK_OEM_3 },
    { KeyBracketL, VK_OEM_4 },
    { KeyBackslash, VK_OEM_5 },
    { KeyBracketR, VK_OEM_6 },
};

int esdwToWinKeyCode(KeyCode keyCode) {
    for (const auto& it : keyCodeMappings)
        if (it.first == keyCode) return it.second;
    return 0;
}

KeyCode winToEsdwKeyCode(int winKeyCode) {
    for (const auto& it : keyCodeMappings)
        if (it.second == winKeyCode) return it.first;
    return KeyUnknown;
}

WindowWin32::WindowWin32(Vec2<I32> size, std::string title) {
    mainLogger.debug("Creating window \"{}\"", title);
    
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    const char className[] = "ESeed Graphics Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = (WNDPROC)windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.cbWndExtra = sizeof(WindowWin32*);

    RegisterClass(&wc);

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = size.x;
    rect.bottom = size.y;

    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW ^ WS_OVERLAPPED, FALSE, NULL);

    hWnd = CreateWindowEx(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL) {
        throw std::runtime_error(
            mainLogger.error("Failed to create native Win32 window")
        );
    }

    ShowWindow(hWnd, SW_SHOW);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
}

WindowWin32::~WindowWin32() {
    if (hWnd) DestroyWindow(hWnd);
}

void WindowWin32::setKeyDownHandler(std::function<void(KeyDownEvent)> handler) {
    keyDownHandler = handler;
}

void WindowWin32::setKeyUpHandler(std::function<void(KeyUpEvent)> handler) {
    keyUpHandler = handler;
}

void WindowWin32::setMouseMoveHandler(std::function<void(MouseMoveEvent)> handler) {
    mouseMoveHandler = handler;
}

bool WindowWin32::getKey(KeyCode keyCode) {
    return GetKeyState(esdwToWinKeyCode(keyCode)) & 0x8000;
}

MousePos WindowWin32::getMouseScreenPos() {
    POINT point;
    if (!GetCursorPos(&point)) throw std::runtime_error("Could not get Win32 mouse position");

    return MousePos { point.x, point.y };
}

MousePos WindowWin32::getMousePos() {
    POINT point;
    if (!GetCursorPos(&point)) throw std::runtime_error("Could not get Win32 mouse position");

    ScreenToClient(hWnd, &point);

    return MousePos { point.x, point.y };
}

void WindowWin32::setMouseScreenPos(MousePos screenPos) {
    SetCursorPos(screenPos.x, screenPos.y);
}

void WindowWin32::setMousePos(MousePos pos) {
    POINT point = { pos.x, pos.y };
    ClientToScreen(hWnd, &point);
    SetCursorPos(point.x, point.y);
}

void WindowWin32::poll() {
    MSG msg;
    while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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

    switch (uMsg) {
    case WM_CLOSE:
        window->closeRequested = true;
        return NULL;
    case WM_DESTROY:
        hWnd = NULL;
        break;
    case WM_KEYDOWN: 
        {
            KeyDownEvent event;
            event.keyCode = winToEsdwKeyCode((int)wParam);
            
            if (window->keyDownHandler) 
                window->keyDownHandler(event);
        }
        break;
    case WM_KEYUP: 
        {
            KeyUpEvent event;
            event.keyCode = winToEsdwKeyCode((int)wParam);
            
            if (window->keyUpHandler) 
                window->keyUpHandler(event);
        }
        break;
    case WM_MOUSEMOVE:
        {
            MouseMoveEvent event;

            event.screenPos.x = GET_X_LPARAM(lParam);
            event.screenPos.y = GET_Y_LPARAM(lParam);

            POINT point = { event.screenPos.x, event.screenPos.y };
            ScreenToClient(hWnd, &point);
            event.pos.x = point.x;
            event.pos.y = point.y;

            if (window->mouseMoveHandler)
                window->mouseMoveHandler(event);
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void WindowWin32::update() {
    RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW);
}

Vec2<I32> WindowWin32::getSize() {
    RECT rect;
    if (GetClientRect(hWnd, &rect)) {
        return { rect.right - rect.left, rect.bottom - rect.top };
    } else { 
        throw std::runtime_error(
            mainLogger.error("Could not get Win32 window size")
        );
    }
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