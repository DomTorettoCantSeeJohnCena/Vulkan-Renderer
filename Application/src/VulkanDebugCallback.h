#pragma once
#include "vulkan/vulkan.h"

class VulkanMessageCallback
{
public:
    static void SetupDebugMessenger(VkInstance instance);
    static void DestroyDebugMessenger(VkInstance instance);

    static VkDebugUtilsMessengerCreateInfoEXT ReturnDebugMessengerCreateInfo();

    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
private:

};