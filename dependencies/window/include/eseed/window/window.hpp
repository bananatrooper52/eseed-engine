#pragma once

#include <eseed/math/types.hpp>
#include <eseed/math/vec.hpp>
#include <eseed/logging/logger.hpp>
#include <vulkan/vulkan.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace esdw {

enum KeyCode {
    KeyUnknown = 0,
    KeyBackspace = 8,
    KeyTab = 9,
    KeyClear = 12,
    KeyEnter = 13,
    KeyShift = 16,
    KeyCtrl = 17,
    KeyAlt = 18,
    KeyPause = 19,
    KeyCapsLock = 20,
    KeyEsc = 27,
    KeySpace = 32,
    KeyPgUp = 33,
    KeyPgDn = 34,
    KeyEnd = 35,
    KeyHome = 36,
    KeyArrowLeft = 37,
    KeyArrowUp = 38,
    KeyArrowRight = 39,
    KeyArrowDown = 40,
    KeyPrntScrn = 44,
    KeyIns = 45,
    KeyDel = 46,
    Key0 = 48,
    Key1 = 49,
    Key2 = 50,
    Key3 = 51,
    Key4 = 52,
    Key5 = 53,
    Key6 = 54,
    Key7 = 55,
    Key8 = 56,
    Key9 = 57,
    KeyA = 65,
    KeyB = 66,
    KeyC = 67,
    KeyD = 68,
    KeyE = 69,
    KeyF = 70,
    KeyG = 71,
    KeyH = 72,
    KeyI = 73,
    KeyJ = 74,
    KeyK = 75,
    KeyL = 76,
    KeyM = 77,
    KeyN = 78,
    KeyO = 79,
    KeyP = 80,
    KeyQ = 81,
    KeyR = 82,
    KeyS = 83,
    KeyT = 84,
    KeyU = 85,
    KeyV = 86,
    KeyW = 87,
    KeyX = 88,
    KeyY = 89,
    KeyZ = 90,
    KeyMetaL = 91,
    KeyMetaR = 92,
    KeySelect = 93,
    KeyNumpad0 = 96,
    KeyNumpad1 = 97,
    KeyNumpad2 = 98,
    KeyNumpad3 = 99,
    KeyNumpad4 = 100,
    KeyNumpad5 = 101,
    KeyNumpad6 = 102,
    KeyNumpad7 = 103,
    KeyNumpad8 = 104,
    KeyNumpad9 = 105,
    KeyMul = 106,
    KeyAdd = 107,
    KeySub = 109,
    KeyDecimal = 110,
    KeyDiv = 111,
    KeyF1 = 112,
    KeyF2 = 113,
    KeyF3 = 114,
    KeyF4 = 115,
    KeyF5 = 116,
    KeyF6 = 117,
    KeyF7 = 118,
    KeyF8 = 119,
    KeyF9 = 120,
    KeyF10 = 121,
    KeyF11 = 122,
    KeyF12 = 123,
    KeyNumLock = 144,
    KeyScrollLock = 145,
    KeySemicolon = 186,
    KeyEqual = 187,
    KeyComma = 188,
    KeyDash = 189,
    KeyPeriod = 190,
    KeySlash = 191,
    KeyGrave = 192,
    KeyBracketL = 219,
    KeyBackslash = 220,
    KeyBracketR = 221
};

class Window {
public:
    // Set event handlers

    virtual void setKeyDownHandler(std::function<void(KeyCode)> handler) = 0;
    virtual void setKeyUpHandler(std::function<void(KeyCode)> handler) = 0;

    // Poll for window events
    virtual void poll() = 0;

    // Check if the close button has been pressed and close is requested
    virtual bool isCloseRequested() = 0;

    // Display newly drawn graphics
    virtual void update() = 0;

    // Get a list of extension names required to create a Vulkan Surface
    virtual std::vector<const char*> getRequiredInstanceExtensionNames() = 0;

    // Create a Vulkan surface
    virtual vk::SurfaceKHR createSurface(vk::Instance instance) = 0;

    // Get dimensions of the window in pixels
    virtual esdm::Vec2<I32> getSize() = 0;
};

// Platform-agnostic function to create a window for the native platform
std::unique_ptr<Window> createWindow(
    esdm::Vec2<I32> size, 
    std::string title
);

}