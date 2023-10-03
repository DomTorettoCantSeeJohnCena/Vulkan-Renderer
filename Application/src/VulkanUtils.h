#pragma once
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

#include <iostream>
#include <vector>
#include <optional>
#include <set>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#undef min
#undef max

extern Window& GetWindow();

const int MAX_FRAMES_IN_FLIGHT = 2;   

#ifdef DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

// Definition of a vector containing Vulkan validation layer names
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Definition of a vector containing Vulkan device extension names
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
    VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME
};

std::vector<VkDynamicState> dynamicStates = { 
    VK_DYNAMIC_STATE_VIEWPORT, 
    VK_DYNAMIC_STATE_SCISSOR 
}; 

// Definition of a C++ struct representing Vulkan queue family indices
struct QueueFamilyIndices
{
    // Index of the graphics queue family
    std::optional<uint32_t> graphicsFamily; 
    // Index of the present queue family (for displaying images)
    std::optional<uint32_t> presentFamily;  

    bool isComplete()
    {
        // Check if both graphicsFamily and presentFamily have valid values
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};


// Definition of a C++ struct encapsulating Vulkan swap chain support details
struct SwapChainSupportDetails
{
    // Surface capabilities, such as image count, dimensions, and usage flags
    VkSurfaceCapabilitiesKHR capabilities; 
    // Supported surface formats, including color space and color format options
    std::vector<VkSurfaceFormatKHR> formats; 
    // Supported presentation modes for displaying images to the screen
    std::vector<VkPresentModeKHR> presentModes; 
};


uint32_t glfwExtensionCount = 0;
const char** glfwExtensions;
uint32_t extensionCount = 0;

#define DEBUG_BREAK __debugbreak();
#define Print(x) spdlog::info(x);
#define CheckForError(x,y) if(x) { spdlog::error(y); DEBUG_BREAK }

namespace Utils
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    void VulkanExtensionSupport()
    {
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "available extensions:\n";

        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) 
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        return true;
    }

    std::vector<const char*> GetRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef DEBUG
        for (int i = 0; i < extensions.size(); i++)
            Print(extensions[i])
#endif

            return extensions;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) 
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) 
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                indices.graphicsFamily = i;
            

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) 
               indices.presentFamily = i;
            

            if (indices.isComplete()) 
                break;
            
            i++;
        }

        return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) 
    {
        uint32_t extensionCount;
        // Query the number of supported device extensions on a specific Vulkan device.
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        // Retrieving the actual properties of the supported device extensions.
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // Create a new set to and copy all required device extensions into it.
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) 
        {
            // Erasing the name of each required extension that is supported.
            requiredExtensions.erase(extension.extensionName);
        }

        // If the set is empty (i.e., all required extensions are supported), the function will return true.
        // Otherwise, if there are missing extensions, it will return false,
        // indicating that not all required extensions are supported.
        return requiredExtensions.empty();
    }
          
    namespace SwapChain
    { 
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            SwapChainSupportDetails details;

            // Query surface capabilities
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            // Query supported surface formats
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            // Query supported presentation modes
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
        {
            // Iterate through the available surface formats
            for (const auto& availableFormat : availableFormats)
            {
                // Check if the desired format and color space are available
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return availableFormat;

            }

            // If desired format is not available, fallback to the first available format
            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
        {
            // Iterate through the available presentation modes
            for (const auto& availablePresentMode : availablePresentModes)
            {
                // Check if the mailbox presentation mode is available
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return availablePresentMode;

            }

            // If mailbox presentation mode is not available, fallback to FIFO mode
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
        {
            // Check if the current extent is already set to a specific value
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }
            else
            {
                // Get the size of the framebuffer associated with the window
                int width, height;
                glfwGetFramebufferSize(GetWindow().GLFW(), &width, &height);

                // Create a VkExtent2D structure with the actual width and height
                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                // Clamp the actual extent to the valid range specified by capabilities
                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                // Return the final chosen extent
                return actualExtent;
            }
        }

    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) 
        {
            SwapChainSupportDetails swapChainSupport = Utils::SwapChain::querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }
}
