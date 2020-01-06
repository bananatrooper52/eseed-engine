#include <eseed/graphics/rendering/rendercontext.hpp>

eseed::graphics::RenderContext::RenderContext()
{
    initInstance();
}

void eseed::graphics::RenderContext::initInstance()
{
    auto instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();
    
    
    vk::ApplicationInfo applicationInfo = {};
    applicationInfo.pApplicationName = "ESeed Engine Game";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "ESeed Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_1;

    vk::InstanceCreateInfo ci = {};
    ci.pApplicationInfo = &applicationInfo;
    ci.enabledExtensionCount = 0;
    ci.enabledLayerCount = 0;

    instance = vk::createInstance(ci);
}