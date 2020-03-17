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
    esdm::Vec2<float> look;
    esdm::Vec3<float> playerPos;
    
    // Settings for main logger
    esdl::mainLogger.setMinLogLevel(esdl::Logger::LogLevelDebug);

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
            float delta = lastTick;
            lastTick = 0;
            tps++;

            if (window->getKey(esdw::KeyEsc)) break;

            look -= esdm::Vec2<float>(window->getMousePos()) / 
                esdm::Vec2<float>(window->getSize()) - 0.5f;
            window->setMousePos(window->getSize() / 2);

            esdm::Vec3<float> dir;
            if (window->getKey(esdw::KeyW)) dir.z--;
            if (window->getKey(esdw::KeyS)) dir.z++;
            if (window->getKey(esdw::KeyA)) dir.x--;
            if (window->getKey(esdw::KeyD)) dir.x++;
            if (window->getKey(esdw::KeyShift)) dir.y--;
            if (window->getKey(esdw::KeySpace)) dir.y++;

            float speed = 10.f;
            esdm::Vec3<float> vel = esdm::Vec3<float>(
                esdm::matmul(esdm::Vec4<float>(dir * speed), esdm::matRotate({ 0, 1, 0 }, look.x))
            );

            playerPos += vel * delta;
        }
        
        if (renderContext.checkFrameAvailable()) {

            fps++;

            window->poll();

            pipeline->setCamera(Camera{ 
                playerPos,
                esdm::matmul(esdm::matRotate({ 1, 0, 0 }, look.y), esdm::matRotate({ 0, 1, 0 }, look.x)),
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