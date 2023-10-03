#include "VulkanDebugCallback.h"
#include <iostream>
#include <vector>

#ifdef DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

#define DEBUG_BREAK __debugbreak();
#define Print(x) std::cout << x << '\n';
#define CheckForError(x,y) if(x) { Print(y) DEBUG_BREAK }

struct VulkanMessageCallbackData
{
    VkDebugUtilsMessengerEXT debugMessenger;
};

#ifdef DEBUG
VulkanMessageCallbackData* s_VulkanMessageCallbackData = new VulkanMessageCallbackData();
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
        func(instance, debugMessenger, pAllocator);
    
}

void VulkanMessageCallback::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr; // Optional
}

VkDebugUtilsMessengerCreateInfoEXT VulkanMessageCallback::ReturnDebugMessengerCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; 

    debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;
    debugCreateInfo.pUserData = nullptr; // Optional

    return debugCreateInfo;
}

void VulkanMessageCallback::SetupDebugMessenger(VkInstance instance)
{
    if (!enableValidationLayers || !instance) 
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo; 
    PopulateDebugMessengerCreateInfo(createInfo); 

    CheckForError(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &s_VulkanMessageCallbackData->debugMessenger) != VK_SUCCESS, "Failed to set up debug messenger!")
}

void VulkanMessageCallback::DestroyDebugMessenger(VkInstance instance)
{
   if (enableValidationLayers)
       DestroyDebugUtilsMessengerEXT(instance, s_VulkanMessageCallbackData->debugMessenger, nullptr);

    delete s_VulkanMessageCallbackData;
    s_VulkanMessageCallbackData = nullptr;
}