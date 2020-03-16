#include <iostream>

#include "gpu/rendercontext.hpp"

#include <eseed/window/window.hpp>
#include <eseed/logging/logger.hpp>
#include <eseed/math/mat.hpp>
#include <eseed/math/matops.hpp>
#include <vector>
#include <chrono>

int main() {
    bool ctrlForward = false;
    bool ctrlBack = false;
    bool ctrlLeft = false;
    bool ctrlRight = false;
    bool ctrlDown = false;
    bool ctrlUp = false;
    esdm::Vec3<float> playerPos;
    
    // Settings for main logger
    esdl::mainLogger.setMinLogLevel(esdl::Logger::LogLevelDebug);

    // Create window
    std::shared_ptr<esdw::Window> window = esdw::createWindow(
        {1366, 768}, 
        "ESeed Engine"
    );
    window->setKeyDownHandler([&](esdw::KeyCode keyCode) {
        switch (keyCode) {
        case esdw::KeyW:
            ctrlForward = true;
            break;
        case esdw::KeyS:
            ctrlBack = true;
            break;
        case esdw::KeyA:
            ctrlLeft = true;
            break;
        case esdw::KeyD:
            ctrlRight = true;
            break;
        case esdw::KeyShift:
            ctrlDown = true;
            break;
        case esdw::KeySpace:
            ctrlUp = true;
            break;
        }
    });

    window->setKeyUpHandler([&](esdw::KeyUpEvent keyCode) {
        switch (keyCode) {
        case esdw::KeyW:
            ctrlForward = false;
            break;
        case esdw::KeyS:
            ctrlBack = false;
            break;
        case esdw::KeyA:
            ctrlLeft = false;
            break;
        case esdw::KeyD:
            ctrlRight = false;
            break;
        case esdw::KeyShift:
            ctrlDown = false;
            break;
        case esdw::KeySpace:
            ctrlUp = false;
            break;
        }
    });

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
            float delta = lastTick;
            lastTick = 0;
            tps++;

            esdm::Vec3<float> dir;
            if (ctrlForward) dir.z--;
            if (ctrlBack) dir.z++;
            if (ctrlLeft) dir.x--;
            if (ctrlRight) dir.x++;
            if (ctrlDown) dir.y--;
            if (ctrlUp) dir.y++;

            float speed = 10.f;
            esdm::Vec3<float> vel = dir * speed;

            playerPos += vel * delta;
        }
        
        if (renderContext.checkFrameAvailable()) {

            fps++;

            window->poll();

            pipeline->setCamera(Camera{ 
                playerPos,
                esdm::matmul(esdm::rotateY(0.f), esdm::rotateX(-.1f)),
                float(window->getSize().x) / float(window->getSize().y),
                0.25f * esdm::pi<float>() * 2.f
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