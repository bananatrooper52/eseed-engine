#include <iostream>

#include <vector>
#include <eseed/window/window.hpp>
#include <eseed/logging/logger.hpp>

#include "gpu/rendercontext.hpp"

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

    RenderContext renderContext(window);
    auto pipeline = renderContext.getRenderPipeline();

    auto objectId = pipeline->addRenderObject(Mesh({
        {{ -1, -1 }, { 1, 0, 0 }},
        {{ 1, -1 }, { 0, 1, 0 }},
        {{ -1, 1 }, { 0, 0, 1 }},
        {{ -1, 1 }, { 0, 0, 1 }},
        {{ 1, -1 }, { 0, 1, 0 }},
        {{ 1, 1 }, { 1, 0, 1 }},
    }));

    auto instanceId = pipeline->addRenderInstance(objectId);

    // Poll window updates and redraw until close is requested
    while (!window->isCloseRequested()) {
        window->poll();

        renderContext.render();

        window->update();
    }

    pipeline->removeRenderInstance(instanceId);
    pipeline->removeRenderObject(objectId);
}