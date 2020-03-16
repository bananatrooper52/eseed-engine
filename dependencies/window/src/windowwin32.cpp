#define VK_USE_PLATFORM_WIN32_KHR

#include <eseed/window/windowwin32.hpp>

#include <eseed/logging/logger.hpp>
#include <windowsx.h>

using namespace esdw;
using namespace esdl;
using namespace esdm;

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

    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW ^ WS_OVERLAPPED, FALSE, 0);

    hWnd = CreateWindowEx(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right,
        rect.bottom,
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
            event.keyCode = findKeyCode(wParam);
            
            if (window->keyDownHandler) 
                window->keyDownHandler(event);
        }
        break;
    case WM_KEYUP: 
        {
            KeyUpEvent event;
            event.keyCode = findKeyCode(wParam);
            
            if (window->keyUpHandler) 
                window->keyUpHandler(event);
        }
        break;
    case WM_MOUSEMOVE:
        {
            MouseMoveEvent event;

            event.screenPos.x = GET_X_LPARAM(lParam);
            event.screenPos.y = GET_Y_LPARAM(lParam);

            POINT point;
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
            mainLogger.error("Could not get win32 window size")
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

KeyCode WindowWin32::findKeyCode(WPARAM wParam) {
    switch (wParam) {
    case VK_BACK: return KeyBackspace;
    case VK_TAB: return KeyTab;
    case VK_CLEAR: return KeyClear;
    case VK_RETURN: return KeyEnter;
    case VK_SHIFT: return KeyShift;
    case VK_CONTROL: return KeyCtrl;
    case VK_MENU: return KeyAlt;
    case VK_PAUSE: return KeyPause;
    case VK_CAPITAL: return KeyCapsLock;
    case VK_ESCAPE: return KeyEsc;
    case VK_SPACE: return KeySpace;
    case VK_PRIOR: return KeyPgUp;
    case VK_NEXT: return KeyPgDn;
    case VK_END: return KeyEnd;
    case VK_HOME: return KeyHome;
    case VK_LEFT: return KeyArrowLeft;
    case VK_UP: return KeyArrowUp;
    case VK_RIGHT: return KeyArrowRight;
    case VK_DOWN: return KeyArrowDown;
    case VK_SELECT: return KeySelect;
    case VK_SNAPSHOT: return KeyPrntScrn;
    case VK_INSERT: return KeyIns;
    case VK_DELETE: return KeyDel;
    case 0x30: return Key0;
    case 0x31: return Key1;
    case 0x32: return Key2;
    case 0x33: return Key3;
    case 0x34: return Key4;
    case 0x35: return Key5;
    case 0x36: return Key6;
    case 0x37: return Key7;
    case 0x38: return Key8;
    case 0x39: return Key9;
    case 0x41: return KeyA;
    case 0x42: return KeyB;
    case 0x43: return KeyC;
    case 0x44: return KeyD;
    case 0x45: return KeyE;
    case 0x46: return KeyF;
    case 0x47: return KeyG;
    case 0x48: return KeyH;
    case 0x49: return KeyI;
    case 0x4a: return KeyJ;
    case 0x4b: return KeyK;
    case 0x4c: return KeyL;
    case 0x4d: return KeyM;
    case 0x4e: return KeyN;
    case 0x4f: return KeyO;
    case 0x50: return KeyP;
    case 0x51: return KeyQ;
    case 0x52: return KeyR;
    case 0x53: return KeyS;
    case 0x54: return KeyT;
    case 0x55: return KeyU;
    case 0x56: return KeyV;
    case 0x57: return KeyW;
    case 0x58: return KeyX;
    case 0x59: return KeyY;
    case 0x5a: return KeyZ;
    case VK_LWIN: return KeyMetaL;
    case VK_RWIN: return KeyMetaR;
    case VK_NUMPAD0: return KeyNumpad0;
    case VK_NUMPAD1: return KeyNumpad1;
    case VK_NUMPAD2: return KeyNumpad2;
    case VK_NUMPAD3: return KeyNumpad3;
    case VK_NUMPAD4: return KeyNumpad4;
    case VK_NUMPAD5: return KeyNumpad5;
    case VK_NUMPAD6: return KeyNumpad6;
    case VK_NUMPAD7: return KeyNumpad7;
    case VK_NUMPAD8: return KeyNumpad8;
    case VK_NUMPAD9: return KeyNumpad9;
    case VK_MULTIPLY: return KeyMul;
    case VK_ADD: return KeyAdd;
    case VK_SUBTRACT: return KeySub;
    case VK_DECIMAL: return KeyDecimal;
    case VK_DIVIDE: return KeyDiv;
    case VK_F1: return KeyF1;
    case VK_F2: return KeyF2;
    case VK_F3: return KeyF3;
    case VK_F4: return KeyF4;
    case VK_F5: return KeyF5;
    case VK_F6: return KeyF6;
    case VK_F7: return KeyF7;
    case VK_F8: return KeyF8;
    case VK_F9: return KeyF9;
    case VK_F10: return KeyF10;
    case VK_F11: return KeyF11;
    case VK_F12: return KeyF12;
    case VK_NUMLOCK: return KeyNumLock;
    case VK_SCROLL: return KeyScrollLock;
    case VK_OEM_1: return KeySemicolon;
    case VK_OEM_PLUS: return KeyEqual;
    case VK_OEM_COMMA: return KeyComma;
    case VK_OEM_MINUS: return KeyDash;
    case VK_OEM_PERIOD: return KeyPeriod;
    case VK_OEM_2: return KeySlash;
    case VK_OEM_3: return KeyGrave;
    case VK_OEM_4: return KeyBracketL;
    case VK_OEM_5: return KeyBackslash;
    case VK_OEM_6: return KeyBracketR;
    default: return KeyUnknown;
    }
}