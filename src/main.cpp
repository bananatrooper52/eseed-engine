#include <iostream>

#include <vector>
#include <eseed/graphics/window/window.hpp>
#include <eseed/graphics/rendering/rendercontext.hpp>
#include <eseed/logging/logger.hpp>

using namespace esd::graphics;
using namespace esd::logging;
using namespace esd::math;

int main() {
    // Settings for main logger
    mainLogger.setMinLogLevel(Logger::eLevelDebug);

    // Create window
    std::shared_ptr<Window> window = createWindow(
        {1366, 768}, 
        "ESeed Engine"
    );

    // Create render context
    RenderContext ctx(window, true);
    ctx.init();

    // Poll window updates and redraw until close is requested
    while (!window->isCloseRequested()) {
        window->poll();
        
        ctx.render();

        window->update();
    }
}