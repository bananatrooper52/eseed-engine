#include <iostream>

#include "gpu/rendercontext.hpp"

#include <eseed/window/window.hpp>
#include <eseed/logging/logger.hpp>
#include <vector>
#include <chrono>

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
        {{ -0.25f, -0.25f }, { 1, 0, 0 }},
        {{ 0.25f, -0.25f }, { 0, 1, 0 }},
        {{ -0.25f, 0.25f }, { 0, 0, 1 }},
        {{ -0.25f, 0.25f }, { 0, 0, 1 }},
        {{ 0.25f, -0.25f }, { 0, 1, 0 }},
        {{ 0.25f, 0.25f }, { 1, 0, 1 }},
    }));

    auto instanceId = pipeline->addRenderInstance(objectId);

    auto lt = std::chrono::high_resolution_clock::now();

    float t = 0;
    float delta = 0;

    // Poll window updates and redraw until close is requested
    while (!window->isCloseRequested()) {

        auto ct = std::chrono::high_resolution_clock::now();
        delta += std::min(std::chrono::duration_cast<std::chrono::nanoseconds>(ct - lt)
            .count() / 1000000000.f, 1.f);
        lt = ct;
        
        if (delta >= 1.f / 120.f) {

            std::cout << delta << std::endl;
            t += delta;

            delta = 1.f / 120.f;

            window->poll();

            pipeline->setCamera(Camera{ sin(t) });
            renderContext.render();
        }
    }
}