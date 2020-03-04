#include <iostream>

#include <vector>
#include <eseed/window/window.hpp>
#include <eseed/logging/logger.hpp>

#include "gpu/gpu.hpp"

using namespace esd::logging;
using namespace esd::math;
using namespace esd::window;

int main() {
    // Settings for main logger
    mainLogger.setMinLogLevel(Logger::eLevelDebug);

    // Create window
    std::shared_ptr<Window> window = createWindow(
        {1366, 768}, 
        "ESeed Engine"
    );

    Gpu gpu(window);

    // Poll window updates and redraw until close is requested
    while (!window->isCloseRequested()) {
        window->poll();

        gpu.render();

        window->update();
    }
}