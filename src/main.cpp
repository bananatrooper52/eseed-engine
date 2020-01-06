#include <iostream>

#include <vector>
#include <eseed/graphics/window/window.hpp>
#include <eseed/logging/format.hpp>
#include <eseed/logging/logger.hpp>

int main() {

    eseed::math::Vec3<F32> a(0.0, 2.0, 0.0);

    a++;

    std::cout << (a) << std::endl;
    
    /* auto window = eseed::graphics::createWindow({ 1366, 768 }, "ESeed Engine");
    
    while (!window->isCloseRequested())
    {
        window->poll();
    } */
}