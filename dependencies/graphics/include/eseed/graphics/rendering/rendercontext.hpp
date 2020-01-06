#pragma once

#include <vulkan/vulkan.hpp>

namespace eseed::graphics
{

class RenderContext
{
public:
    RenderContext();

private:
    vk::Instance instance;

    void initInstance();
};

}