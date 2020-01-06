#pragma once

#include <memory>
#include <string>
#include <eseed/math/types.hpp>
#include <eseed/math/vec.hpp>

namespace eseed::graphics
{

class Window
{
public:
    virtual void poll() = 0;
    virtual bool isCloseRequested() = 0;
};

std::unique_ptr<Window> createWindow(eseed::math::Vec2<I32> size, std::string title);

} // namespace eseed::graphics