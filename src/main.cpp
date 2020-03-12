#include <iostream>

#include "gpu/rendercontext.hpp"

#include <eseed/window/window.hpp>
#include <eseed/logging/logger.hpp>
#include <eseed/math/mat.hpp>
#include <eseed/math/matops.hpp>
#include <vector>
#include <chrono>

int main() {
    // Settings for main logger
    esdl::mainLogger.setMinLogLevel(esdl::Logger::eLevelDebug);

    // Create window
    std::shared_ptr<esdw::Window> window = esdw::createWindow(
        {1366, 768}, 
        "ESeed Engine"
    );

    RenderContext renderContext(window);
    auto pipeline = renderContext.getRenderPipeline();

    auto objectId = pipeline->addRenderObject(Mesh({
        {{ -1.f, -1.f }, { 1, 0, 0 }},
        {{ 1.f, -1.f }, { 0, 1, 0 }},
        {{ -1.f, 1.f }, { 0, 0, 1 }},
        {{ -1.f, 1.f }, { 0, 0, 1 }},
        {{ 1.f, -1.f }, { 0, 1, 0 }},
        {{ 1.f, 1.f }, { 1, 1, 1 }},
    }));

    auto instanceId = pipeline->addRenderInstance(objectId);

    auto lt = std::chrono::high_resolution_clock::now();

    float t = 0;
    float lastTick = 0;
    float lastSecond = 0;

    size_t tps = 0;
    size_t fps = 0;

    // Poll window updates and redraw until close is requested
    while (!window->isCloseRequested()) {

        auto ct = std::chrono::high_resolution_clock::now();
        float iterDelta = std::min(std::chrono::duration_cast<std::chrono::nanoseconds>(ct - lt)
            .count() / 1000000000.f, 1.f);
        lt = ct;

        lastTick += iterDelta;
        lastSecond += iterDelta;
        t += iterDelta;

        if (lastTick >= 1.f / 60.f) {
            tps++;
        }
        
        if (renderContext.checkFrameAvailable()) {

            fps++;

            window->poll();

            pipeline->setCamera(Camera{ 
                esdm::Vec3<float>(0, 0, t * 16.f),
                esdm::rotateX(0.5f) * esdm::rotateY(1.f),
                float(window->getSize().x) / float(window->getSize().y)
            });
            renderContext.render();
        }

        if (lastSecond >= 1.f) {
            std::cout << fps << std::endl;
            lastSecond = 0;
            fps = 0;
        }
    }
}