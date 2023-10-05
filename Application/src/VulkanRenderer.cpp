#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "VulkanRenderer.h"
#include "VulkanDebugCallback.h"
#include "Window.h"
#include "VulkanUtils.h"
#include "Shader.h"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <vector>
#include <array>
#include <memory>

struct Vertex
{
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }

    glm::vec2 pos;
    glm::vec3 color;
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct VulkanData
{
    VkInstance instance;
    VkSurfaceKHR surface;

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    /*This member represents the index of the queue family that supports graphics commands.
    Graphics commands are used for rendering operations, such as drawing triangles,
    rendering textures, and setting up the graphics pipeline.*/
    VkQueue graphicsQueue;

    /*This member represents the index of the queue family that supports presentation to the surface.
    It is required for rendering to the window or surface on the screen.*/
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;


    //ImGuiStuff

    VkCommandPool imGuiCommandPool;
    VkDescriptorPool imGuiDescriptorPool;
    VkRenderPass imGuiRenderPass;
    std::vector<VkFramebuffer> imGuiFramebuffers;
    VkCommandBuffer imGuiCommandBuffer;

    std::vector<std::string> successQueue;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer; 
    VkDeviceMemory indexBufferMemory; 

    VkDescriptorSetLayout descriptorSetLayout; 

    std::vector<VkBuffer> uniformBuffers; 
    std::vector<VkDeviceMemory> uniformBuffersMemory; 
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets; 
    
    //ImGuiViewportvRendering 

    std::vector<VkImage> viewportImages;
    std::vector<VkDeviceMemory> imageMemory;
    std::vector<VkImageView> viewportImageViews;
    VkRenderPass viewportRenderPass;
    VkPipeline viewportPipeline;
    VkCommandPool viewportCommandPool;
    std::vector<VkFramebuffer> viewportFramebuffers;
    std::vector<VkCommandBuffer> viewportCommandBuffers;

    VkSampler textureSampler;    
    UniformBufferObject ubo{};  
    bool EnableImGui = true;

    float f = 0.0f;
};

static VulkanData s_VulkanData;

uint32_t currentFrame = 0; 

void VulkanRenderer::VulkanInit()
{
    CreateInstance();
    CreateSurface();
    PhysicalDevice();  
    CreateLogicalDevice(); 
    CreateSwapChain(); 
    CreateImageViews(); 

  
    CreateDescriptorSetLayout();   
    CreateGraphicsPipeline(); 

      
    CreateFramebuffers(); 
    CreateCommandPool();  
    CreateVertexBuffer();  
    CreateIndexBuffer();

    CreateUniformBuffers();
    CreateDescriptorPool();    
    CreateDescriptorSets(); 

    CreateCommandBuffer(); 
    CreateSyncObjects(); 

    if (s_VulkanData.EnableImGui)
        InitImGui();


    for (int i = 0; i < s_VulkanData.successQueue.size(); i++)
    {
        Print(s_VulkanData.successQueue[i])
    }

    s_VulkanData.successQueue.clear();
}

void VulkanRenderer::CreateInstance()
{
    Utils::VulkanExtensionSupport();

    CheckForError(enableValidationLayers && !Utils::checkValidationLayerSupport(), "Validation layers requested, but not available!")

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine One";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Retrieving required GLFW extensions and count for window creation. 
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); 
    auto extensions = Utils::GetRequiredExtensions(); 

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

   
    if (enableValidationLayers)
    { 
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&VulkanMessageCallback::ReturnDebugMessengerCreateInfo(); 
    } 
    else 
    {
        createInfo.enabledLayerCount = 0; 
        createInfo.pNext = nullptr; 
    }

    CheckForError(vkCreateInstance(&createInfo, nullptr, &s_VulkanData.instance) != VK_SUCCESS, "Failed to create Instance!")
    s_VulkanData.successQueue.push_back("Instance successfully created!");
    if (enableValidationLayers)
        VulkanMessageCallback::SetupDebugMessenger(s_VulkanData.instance);
}

void VulkanRenderer::PhysicalDevice()
{
    // Initialize device count to zero
    uint32_t deviceCount = 0;

    // Query the number of available physical devices
    vkEnumeratePhysicalDevices(s_VulkanData.instance, &deviceCount, nullptr);

    // Check if no devices with Vulkan support are found
    CheckForError(deviceCount == 0, "Failed to find GPUs with Vulkan support!")

        // Resize the vector to hold available physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);

    // Populate the vector with available physical devices
    vkEnumeratePhysicalDevices(s_VulkanData.instance, &deviceCount, devices.data());

    // Iterate through each physical device to find a suitable one
    for (const auto& device : devices)
    {
        // Check if the current device is suitable for use
        if (Utils::isDeviceSuitable(device, s_VulkanData.surface))
        {
            // Assign the suitable physical device
            s_VulkanData.physicalDevice = device;
            break; 
        }
    }

    CheckForError(s_VulkanData.physicalDevice == VK_NULL_HANDLE, "Failed to find a suitable GPU!")
    s_VulkanData.successQueue.push_back("Suitable GPU found!");

}


void VulkanRenderer::CreateLogicalDevice()
{
    // Specify queue priority for device queues
    float queuePriority = 1.0f;

    // Find queue families with necessary capabilities
    Utils::QueueFamilyIndices indices = Utils::findQueueFamilies(s_VulkanData.physicalDevice, s_VulkanData.surface);

    // Create a vector to hold device queue create info structures
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // Create a set to store unique queue family indices
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    // Iterate through each unique queue family and create queue create info
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify device features that the logical device will support
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create info structure for the logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;

    // Specify device extensions that the logical device will use
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Check if validation layers are enabled
    if (enableValidationLayers)
    {
        // Specify validation layers for the logical device
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        // No validation layers if not enabled
        createInfo.enabledLayerCount = 0;
    }

    CheckForError(vkCreateDevice(s_VulkanData.physicalDevice, &createInfo, nullptr, &s_VulkanData.device) != VK_SUCCESS, "Failed to create Logical Device!")

    vkGetDeviceQueue(s_VulkanData.device, indices.graphicsFamily.value(), 0, &s_VulkanData.graphicsQueue);
    s_VulkanData.successQueue.push_back("Successfully retrieved Graphics Queue Family!");

    // Get the present queue from the logical device
    vkGetDeviceQueue(s_VulkanData.device, indices.presentFamily.value(), 0, &s_VulkanData.presentQueue);
    s_VulkanData.successQueue.push_back("Successfully retrieved Present Queue Family!");
}


void VulkanRenderer::CreateSurface()
{
    auto window = GetWindow().GLFW();

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);

    CheckForError(vkCreateWin32SurfaceKHR(s_VulkanData.instance, &createInfo, nullptr, &s_VulkanData.surface) != VK_SUCCESS, "Failed to create window surface!")  
    s_VulkanData.successQueue.push_back("Surface successfully created!");
}

void VulkanRenderer::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = Utils::SwapChain::querySwapChainSupport(s_VulkanData.physicalDevice, s_VulkanData.surface);

    VkSurfaceFormatKHR surfaceFormat = Utils::SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = Utils::SwapChain::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = Utils::SwapChain::chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
        imageCount = swapChainSupport.capabilities.maxImageCount;
    
    VkSwapchainCreateInfoKHR createInfo{}; 
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; 
    createInfo.surface = s_VulkanData.surface; 
    createInfo.minImageCount = imageCount; 
    createInfo.imageFormat = surfaceFormat.format; 
    createInfo.imageColorSpace = surfaceFormat.colorSpace; 
    createInfo.imageExtent = extent; 
    createInfo.imageArrayLayers = 1; 
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;  

    CheckForError(vkCreateSwapchainKHR(s_VulkanData.device, &createInfo, nullptr, &s_VulkanData.swapChain) != VK_SUCCESS, "Failed to create swap chain!")
    s_VulkanData.successQueue.push_back("Swap Chain successfully created!");

    // Call to retrieve the number of images in the swap chain
    vkGetSwapchainImagesKHR(s_VulkanData.device, s_VulkanData.swapChain, &imageCount, nullptr);

    // Resize the swapChainImages vector to hold the retrieved number of images
    s_VulkanData.swapChainImages.resize(imageCount); 

    // Call to actually retrieve the swap chain images and store them in the swapChainImages vector
    vkGetSwapchainImagesKHR(s_VulkanData.device, s_VulkanData.swapChain, &imageCount, s_VulkanData.swapChainImages.data()); 

    s_VulkanData.swapChainImageFormat = surfaceFormat.format;
    s_VulkanData.swapChainExtent = extent;
}

void VulkanRenderer::CreateImageViews()
{
    // Resize the array to hold the same number of image views as swap chain images
    s_VulkanData.swapChainImageViews.resize(s_VulkanData.swapChainImages.size());

    // Iterate through each swap chain image and create an image view for it
    for (size_t i = 0; i < s_VulkanData.swapChainImages.size(); i++)
    {
        // Create a structure to hold the image view creation information
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // The image this view will be created for
        createInfo.image = s_VulkanData.swapChainImages[i];
        // The type of the image view (2D in this case)
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; 
        // The format of the image view
        createInfo.format = s_VulkanData.swapChainImageFormat; 
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Define how the image view will be used (color aspect, mip levels, array layers)
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;


        CheckForError(vkCreateImageView(s_VulkanData.device, &createInfo, nullptr, &s_VulkanData.swapChainImageViews[i]) != VK_SUCCESS, "Failed to create Image View!");      
        s_VulkanData.successQueue.push_back("Image View successfully created!");
    }
}

void VulkanRenderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{}; 
    colorAttachment.format = s_VulkanData.swapChainImageFormat; // Format of the color attachment 
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples per pixel 
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Load operation for color data 
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store operation for color data 
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Load operation for stencil data 
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Store operation for stencil data 
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Initial layout of the attachment 
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout of the 
                                                                                                                                               
    VkAttachmentReference colorAttachmentRef{};                                                                                                
    colorAttachmentRef.attachment = 0; // Index of the attachment in the render pass                                                           
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout of the attachment during this subpass                      
                                                                                                                                               
    VkSubpassDescription subpass{};                                                                                                            
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Bind point for this subpass                                                
    subpass.colorAttachmentCount = 1; // Number of color attachments in this subpass                                                           
    subpass.pColorAttachments = &colorAttachmentRef; // Array of color attachment references                                                   
                                                                                                                                               
    VkSubpassDependency dependency{}; 
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; 
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 

    VkRenderPassCreateInfo renderPassInfo{};                                                                                                  
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // Type of the structure                                                
    renderPassInfo.attachmentCount = 1; // Number of attachments                                                                              
    renderPassInfo.pAttachments = &colorAttachment; // Array of attachment descriptions                                                       
    renderPassInfo.subpassCount = 1; // Number of subpasses                                                                                   
    renderPassInfo.pSubpasses = &subpass; // Array of subpass descriptions                       
    renderPassInfo.dependencyCount = 1;                                                          
    renderPassInfo.pDependencies = &dependency;                                                  
                         
    CheckForError(vkCreateRenderPass(s_VulkanData.device, &renderPassInfo, nullptr, &s_VulkanData.renderPass) != VK_SUCCESS, "Failed to create Render Pass!")
    s_VulkanData.successQueue.push_back("Render Pass successfully created!");
}

void VulkanRenderer::CreateGraphicsPipeline()
{
    CreateRenderPass(); 
    Shader::Initialize(s_VulkanData.device);  
    Shader* shader = new Shader(); 

    // Create dynamic state info for the pipeline
    VkPipelineDynamicStateCreateInfo dynamicState{}; 
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO; 
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()); // Number of dynamic states 
    dynamicState.pDynamicStates = dynamicStates.data(); // Array of dynamic state types 

    auto bindingDescription = Vertex::getBindingDescription();  
    auto attributeDescriptions = Vertex::getAttributeDescriptions();  

    // Create vertex input state info for the pipeline
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; 
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
     
    // Create input assembly state info for the pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{}; 
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Primitive assembly topology 
    inputAssembly.primitiveRestartEnable = VK_FALSE; // Allow restarting of primitive assembly (e.g., strip restart) 
    
    
    // Create a viewport for the pipeline
    VkViewport viewport{}; 
    viewport.x = 0.0f; // X coordinate of the lower-left corner of the viewport 
    viewport.y = 0.0f; // Y coordinate of the lower-left corner of the viewport 
    viewport.width = static_cast<float>(s_VulkanData.swapChainExtent.width); // Width of the viewport 
    viewport.height = static_cast<float>(s_VulkanData.swapChainExtent.height); // Height of the viewport 
    viewport.minDepth = 0.0f; // Minimum depth value of the viewport (NDC) 
    viewport.maxDepth = 1.0f; // Maximum depth value of the viewport (NDC) 

    VkRect2D scissor{}; 
    scissor.offset = { 0, 0 }; // Top-left corner offset of the scissor rectangle 
    scissor.extent = s_VulkanData.swapChainExtent; // Extent (size) of the scissor rectangle 

    VkPipelineViewportStateCreateInfo viewportState{}; 
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; 
    viewportState.viewportCount = 1; // Number of viewports 
    viewportState.scissorCount = 1; // Number of scissor rectangles 

    // Create rasterization state info for the pipeline
    VkPipelineRasterizationStateCreateInfo rasterizer{}; 
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; 
    rasterizer.depthClampEnable = VK_FALSE; // Disable depth clamping 
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Enable rasterization of primitives 
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Fill the interiors of polygons 
    rasterizer.lineWidth = 1.0f; // Set line width for line primitives 
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; 
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; 
    rasterizer.depthBiasEnable = VK_FALSE; // Disable depth bias 
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional depth bias constant factor 
    rasterizer.depthBiasClamp = 0.0f; // Optional depth bias clamp 
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional depth bias slope factor 

    // Create multisample state info for the pipeline
    VkPipelineMultisampleStateCreateInfo multisampling{}; 
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; 
    multisampling.sampleShadingEnable = VK_FALSE; // Disable sample shading 
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Use 1 sample per pixel 
    multisampling.minSampleShading = 1.0f; // Optional: Minimum fraction of samples shaded (disabled here) 
    multisampling.pSampleMask = nullptr; // Optional: Sample mask (disabled here) 
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional: Disable alpha to coverage 
    multisampling.alphaToOneEnable = VK_FALSE; // Optional: Disable alpha to one 

    // Create color blend attachment state for the pipeline
    VkPipelineColorBlendAttachmentState colorBlendAttachment{}; 
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
    colorBlendAttachment.blendEnable = VK_FALSE; // Disable color blending 
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional: Source color blending factor 
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional: Destination color blending factor 
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional: Color blending operation 
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional: Source alpha blending factor 
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional: Destination alpha blending factor 
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional: Alpha blending operation                                                                   

    // Create color blend state info for the pipeline
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // Disable logical operations
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional: Logical operation (disabled here)
    colorBlending.attachmentCount = 1; // Number of color attachments
    colorBlending.pAttachments = &colorBlendAttachment; // Color blend attachment state
    colorBlending.blendConstants[0] = 0.0f; // Optional: Blend constants for R component
    colorBlending.blendConstants[1] = 0.0f; // Optional: Blend constants for G component
    colorBlending.blendConstants[2] = 0.0f; // Optional: Blend constants for B component
    colorBlending.blendConstants[3] = 0.0f; // Optional: Blend constants for A component

    // Create pipeline layout info
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Descriptor layout 
    pipelineLayoutInfo.pSetLayouts = &s_VulkanData.descriptorSetLayout; // Descriptor layout 
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional: Number of push constant ranges 
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional: Push constant ranges 

    CheckForError(vkCreatePipelineLayout(s_VulkanData.device, &pipelineLayoutInfo, nullptr, &s_VulkanData.pipelineLayout) != VK_SUCCESS, "Failed to create Pipeline Layout!")
    s_VulkanData.successQueue.push_back("Pipeline Layout successfully created!");
      
    // Declare and initialize a VkGraphicsPipelineCreateInfo structure
    VkGraphicsPipelineCreateInfo pipelineInfo {};

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; // Set the type of the structure
    pipelineInfo.stageCount = 2; // Specify the number of shader stages in the pipeline
    pipelineInfo.pStages = shader->GetShaderStages().data(); // Get the array of shader stages and assign it to pStages
    pipelineInfo.pVertexInputState = &vertexInputInfo; // Set the vertex input state configuration
    pipelineInfo.pInputAssemblyState = &inputAssembly; // Set the input assembly state configuration
    pipelineInfo.pViewportState = &viewportState; // Set the viewport state configuration                                                           
    pipelineInfo.pRasterizationState = &rasterizer; // Set the rasterization state configuration                                                    
    pipelineInfo.pMultisampleState = &multisampling; // Set the multisample state configuration                                                     
    pipelineInfo.pDepthStencilState = nullptr; // Set the depth and stencil state configuration (set to nullptr for not using them)                 
    pipelineInfo.pColorBlendState = &colorBlending; // Set the color blend state configuration                                                      
    pipelineInfo.pDynamicState = &dynamicState; // Set the dynamic state configuration                                                                                                                     
                                                                                                                                                                                                           
    pipelineInfo.layout = s_VulkanData.pipelineLayout; // Set the pipeline layout (assuming s_VulkanData.pipelineLayout is defined)                                                                      
    pipelineInfo.renderPass = s_VulkanData.renderPass; // Set the render pass (assuming s_VulkanData.renderPass is defined)                                                                              
    pipelineInfo.subpass = 0; // Specify the subpass index                                                                                                                                                 
                                                                                                                                                                                                           
                                                                                                                                                                                                           
    CheckForError(vkCreateGraphicsPipelines(s_VulkanData.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &s_VulkanData.graphicsPipeline) != VK_SUCCESS, "Failed to create Graphics Pipeline!") 
    s_VulkanData.successQueue.push_back("Graphics Pipeline successfully created!");
       
    delete shader;
    shader = nullptr;
}

void VulkanRenderer::CreateFramebuffers()
{                                                                                                                                                           
    s_VulkanData.swapChainFramebuffers.resize(s_VulkanData.swapChainImageViews.size());                                                                   
    s_VulkanData.successQueue.push_back("Image Views:" + std::to_string(s_VulkanData.swapChainImageViews.size()));
                                                                                                                                                            
        // Loop through each swap chain image view                                                                                                          
        for (size_t i = 0; i < s_VulkanData.swapChainImageViews.size(); i++) {                                                                             
            // Create an array of attachments containing the current swap chain image view                                                                  
            VkImageView attachments[] = {                                                                                                                   
                s_VulkanData.swapChainImageViews[i]                                                                                                        
            };                                                                                                                                              
                                                                                                                                                            
            // Create a VkFramebufferCreateInfo struct for setting up a framebuffer                                                                         
            VkFramebufferCreateInfo framebufferInfo{};                                                                                                      
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; // Specify the type of the struct                                            
            framebufferInfo.renderPass = s_VulkanData.renderPass; // Specify the render pass associated with this framebuffer                              
            framebufferInfo.attachmentCount = 1; // Specify the number of attachments in the framebuffer                                                    
            framebufferInfo.pAttachments = attachments; // Specify the array of attachments                                                                 
            framebufferInfo.width = s_VulkanData.swapChainExtent.width; // Specify the width of the framebuffer                                            
            framebufferInfo.height = s_VulkanData.swapChainExtent.height; // Specify the height of the framebuffer                                         
            framebufferInfo.layers = 1; // Specify the number of layers in the framebuffer (1 for most cases)   

            CheckForError(vkCreateFramebuffer(s_VulkanData.device, &framebufferInfo, nullptr, &s_VulkanData.swapChainFramebuffers[i]) != VK_SUCCESS, "Failed to create framebuffer!") 
            s_VulkanData.successQueue.push_back("Framebuffer successfully created!");
        }
}

void VulkanRenderer::CreateCommandPool()
{
    // Assuming the 'findQueueFamilies' function returns a 'QueueFamilyIndices' object
    Utils::QueueFamilyIndices queueFamilyIndices = Utils::findQueueFamilies(s_VulkanData.physicalDevice, s_VulkanData.surface);                    
                                                                                                                                                     
    // Create a VkCommandPoolCreateInfo struct for setting up a command pool                                                                         
    VkCommandPoolCreateInfo poolInfo{};                                                                                                              
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; // Specify the type of the struct                                                   
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Specify command pool creation flags                                         
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // Specify the queue family index                                         
                                                                                                                                                     
    // 'poolInfo' contains the information required to create a Vulkan command pool  
    CheckForError(vkCreateCommandPool(s_VulkanData.device, &poolInfo, nullptr, &s_VulkanData.commandPool) != VK_SUCCESS, "Failed to create Command Pool!")
    s_VulkanData.successQueue.push_back("Command Pool successfully created!");                                                                                                                                                   
}

void VulkanRenderer::CreateCommandBuffer()
{                                                              
    s_VulkanData.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT); 

    // Create a VkCommandBufferAllocateInfo struct for allocating command buffers                                        
    VkCommandBufferAllocateInfo allocInfo{};                                                                             
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // Specify the type of the struct                  
    allocInfo.commandPool = s_VulkanData.commandPool; // Specify the command pool from which to allocate                
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Specify the command buffer level                               
    allocInfo.commandBufferCount = (uint32_t)s_VulkanData.commandBuffers.size();  // Specify the number of command buffers to allocate                               
                                                                                                                         
    // 'allocInfo' contains the information required to allocate a Vulkan command buffer                                 
                                                                                                                         
    CheckForError(vkAllocateCommandBuffers(s_VulkanData.device, &allocInfo, s_VulkanData.commandBuffers.data()) != VK_SUCCESS, "Failed to create Command Buffer!")
    s_VulkanData.successQueue.push_back("Command Buffer successfully created!");
}

void VulkanRenderer::CreateSyncObjects() 
{
    s_VulkanData.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT); 
    s_VulkanData.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT); 
    s_VulkanData.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT); 

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(s_VulkanData.device, &semaphoreInfo, nullptr, &s_VulkanData.imageAvailableSemaphores[i]) != VK_SUCCESS || 
            vkCreateSemaphore(s_VulkanData.device, &semaphoreInfo, nullptr, &s_VulkanData.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(s_VulkanData.device, &fenceInfo, nullptr, &s_VulkanData.inFlightFences[i]) != VK_SUCCESS) 
        {

            CheckForError(true, "Failed to create Synchronization Objects for a frame!")
        }

        s_VulkanData.successQueue.push_back("Semaphore for available image successfully created!");
        s_VulkanData.successQueue.push_back("Semaphore for rendered finished successfully created!");
        s_VulkanData.successQueue.push_back("Fence successfully created!");
    }
}

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(s_VulkanData.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    CheckForError(true, "Failed to find suitable memory type!")
}

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CheckForError(vkCreateBuffer(s_VulkanData.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS, "Failed to create Buffer!")

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(s_VulkanData.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    CheckForError(vkAllocateMemory(s_VulkanData.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS, "Failed to allocate Buffer Memory!")
    vkBindBufferMemory(s_VulkanData.device, buffer, bufferMemory, 0);
}

void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = s_VulkanData.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(s_VulkanData.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{}; 
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; 
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

    vkBeginCommandBuffer(commandBuffer, &beginInfo); 

    VkBufferCopy copyRegion{}; 
    copyRegion.size = size; 
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer); 

    VkSubmitInfo submitInfo{}; 
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; 
    submitInfo.commandBufferCount = 1; 
    submitInfo.pCommandBuffers = &commandBuffer; 

    vkQueueSubmit(s_VulkanData.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(s_VulkanData.graphicsQueue); 

    vkFreeCommandBuffers(s_VulkanData.device, s_VulkanData.commandPool, 1, &commandBuffer);
}

void VulkanRenderer::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer; 
    VkDeviceMemory stagingBufferMemory;

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory); 

    void* data;
    vkMapMemory(s_VulkanData.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize); 
    vkUnmapMemory(s_VulkanData.device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s_VulkanData.vertexBuffer, s_VulkanData.vertexBufferMemory);
    CopyBuffer(stagingBuffer, s_VulkanData.vertexBuffer, bufferSize); 

    vkDestroyBuffer(s_VulkanData.device, stagingBuffer, nullptr); 
    vkFreeMemory(s_VulkanData.device, stagingBufferMemory, nullptr); 
}

void VulkanRenderer::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(s_VulkanData.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(s_VulkanData.device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s_VulkanData.indexBuffer, s_VulkanData.indexBufferMemory); 

    CopyBuffer(stagingBuffer, s_VulkanData.indexBuffer, bufferSize);

    vkDestroyBuffer(s_VulkanData.device, stagingBuffer, nullptr);
    vkFreeMemory(s_VulkanData.device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional 

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    CheckForError(vkCreateDescriptorSetLayout(s_VulkanData.device, &layoutInfo, nullptr, &s_VulkanData.descriptorSetLayout) != VK_SUCCESS, "Failed to create descriptor set layout!")
}

void VulkanRenderer::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    s_VulkanData.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    s_VulkanData.uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    s_VulkanData.uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, s_VulkanData.uniformBuffers[i], s_VulkanData.uniformBuffersMemory[i]);
        vkMapMemory(s_VulkanData.device, s_VulkanData.uniformBuffersMemory[i], 0, bufferSize, 0, &s_VulkanData.uniformBuffersMapped[i]);
    }
}

void VulkanRenderer::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    CheckForError(vkCreateDescriptorPool(s_VulkanData.device, &poolInfo, nullptr, &s_VulkanData.descriptorPool) != VK_SUCCESS, "Failed to create descriptor pool!")
}

void VulkanRenderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, s_VulkanData.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = s_VulkanData.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    s_VulkanData.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    CheckForError(vkAllocateDescriptorSets(s_VulkanData.device, &allocInfo, s_VulkanData.descriptorSets.data()) != VK_SUCCESS, "Failed to allocate descriptor sets!")

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = s_VulkanData.uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
            
            //VkDescriptorImageInfo imageInfo{};
            //imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            //imageInfo.imageView = s_VulkanData.textureImageView;
            //imageInfo.sampler = s_VulkanData.textureSampler;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = s_VulkanData.descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional 
            descriptorWrite.pTexelBufferView = nullptr; // Optional 

            vkUpdateDescriptorSets(s_VulkanData.device, 1, &descriptorWrite, 0, nullptr);
        }

}

VkCommandBuffer beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = s_VulkanData.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(s_VulkanData.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}    

void endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(s_VulkanData.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(s_VulkanData.graphicsQueue);

    vkFreeCommandBuffers(s_VulkanData.device, s_VulkanData.commandPool, 1, &commandBuffer);
}

void VulkanRenderer::InitImGui()
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    // Create the descriptor pool
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Adjust flags as needed
    pool_info.maxSets = 1000; // Maximum number of descriptor sets
    pool_info.poolSizeCount = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.pPoolSizes = pool_sizes;

    CheckForError(vkCreateDescriptorPool(s_VulkanData.device, &pool_info, nullptr, &s_VulkanData.imGuiDescriptorPool) != VK_SUCCESS, "Failed to create ImGui Descriptor Pool!")
    
    VkAttachmentDescription attachment = {};
    attachment.format = s_VulkanData.swapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    CheckForError(vkCreateRenderPass(s_VulkanData.device, &info, nullptr, &s_VulkanData.imGuiRenderPass) != VK_SUCCESS, "Failed to create ImGui Render Pass!")
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext();  
    ImGuiIO& io = ImGui::GetIO(); (void)io; 
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark(); 

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(GetWindow().GLFW(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = s_VulkanData.instance;
    init_info.PhysicalDevice = s_VulkanData.physicalDevice;
    init_info.Device = s_VulkanData.device;
    init_info.QueueFamily = Utils::findQueueFamilies(s_VulkanData.physicalDevice, s_VulkanData.surface).graphicsFamily.value();
    init_info.Queue = s_VulkanData.graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = s_VulkanData.imGuiDescriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = s_VulkanData.swapChainImages.size();
    init_info.ImageCount = s_VulkanData.swapChainImages.size(); 
    ImGui_ImplVulkan_Init(&init_info, s_VulkanData.imGuiRenderPass);

    VkCommandBuffer command_buffer = beginSingleTimeCommands();   
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);  
    endSingleTimeCommands(command_buffer);  

    /// Assumption 
    
    s_VulkanData.imGuiFramebuffers.resize(s_VulkanData.swapChainImageViews.size());

    for (uint32_t i = 0; i < s_VulkanData.swapChainImageViews.size(); i++) 
    {
        VkImageView attachment[] = { s_VulkanData.swapChainImageViews[i] }; // Set the correct image view

        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = s_VulkanData.imGuiRenderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment; // Attach the correct image view
        info.width = s_VulkanData.swapChainExtent.width;
        info.height = s_VulkanData.swapChainExtent.height;
        info.layers = 1;

        CheckForError(vkCreateFramebuffer(s_VulkanData.device, &info, nullptr, &s_VulkanData.imGuiFramebuffers[i]) != VK_SUCCESS, "Failed to create ImGui Framebuffer!");
    }


    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = Utils::findQueueFamilies(s_VulkanData.physicalDevice, s_VulkanData.surface).graphicsFamily.value();
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CheckForError(vkCreateCommandPool(s_VulkanData.device, &commandPoolCreateInfo, nullptr, &s_VulkanData.imGuiCommandPool) != VK_SUCCESS, "Could not create graphics ImGui Command Pool!")

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = s_VulkanData.imGuiCommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(s_VulkanData.device, &commandBufferAllocateInfo, &s_VulkanData.imGuiCommandBuffer);

    CreateViewportTextureSampler(); 
}

void VulkanRenderer::CreateViewportImages()
{
    s_VulkanData.viewportImages.resize(s_VulkanData.swapChainImages.size()); 
    s_VulkanData.imageMemory.resize(s_VulkanData.swapChainImages.size());

    for (uint32_t i = 0; i < s_VulkanData.swapChainImages.size(); i++)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; 
        imageInfo.imageType = VK_IMAGE_TYPE_2D; 
        imageInfo.format = VK_FORMAT_B8G8R8A8_SRGB; 
        imageInfo.extent.width = s_VulkanData.swapChainExtent.width; 
        imageInfo.extent.height = s_VulkanData.swapChainExtent.height; 
        imageInfo.extent.depth = 1;  
        imageInfo.arrayLayers = 1;  
        imageInfo.mipLevels = 1; 
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; 
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR; 
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; 

        CheckForError(vkCreateImage(s_VulkanData.device, &imageInfo, nullptr, &s_VulkanData.viewportImages[i]) != VK_SUCCESS, "Failed to create Image!");

        VkMemoryRequirements memRequirements; 
        vkGetImageMemoryRequirements(s_VulkanData.device, s_VulkanData.viewportImages[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{} ;
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; 
        allocInfo.allocationSize = memRequirements.size; 
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 
        CheckForError(vkAllocateMemory(s_VulkanData.device, &allocInfo, nullptr, &s_VulkanData.imageMemory[i]) != VK_SUCCESS, "Failed to allocate Image Memory!")
        vkBindImageMemory(s_VulkanData.device, s_VulkanData.viewportImages[i], s_VulkanData.imageMemory[i], 0);
    }
}

void VulkanRenderer::CreateViewportImageViews() 
{
    s_VulkanData.swapChainImageViews.resize(s_VulkanData.swapChainImages.size()); 

    for (size_t i = 0; i < s_VulkanData.swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{}; 
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; 
        createInfo.image = s_VulkanData.viewportImages[i];  
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; 
        createInfo.format = s_VulkanData.swapChainImageFormat;    
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
        createInfo.subresourceRange.baseMipLevel = 0;   
        createInfo.subresourceRange.levelCount = 1;  
        createInfo.subresourceRange.baseArrayLayer = 0;  
        createInfo.subresourceRange.layerCount = 1; 

        CheckForError(vkCreateImageView(s_VulkanData.device, &createInfo, nullptr, &s_VulkanData.viewportImageViews[i]) != VK_SUCCESS, "Failed to create Viewport Image Views!");
        s_VulkanData.successQueue.push_back("Viewport Image Views successfully created!");
    }
}

void VulkanRenderer::CreateViewportRenderPass()
{

}

void VulkanRenderer::CreateViewportPipeline()
{

}

void VulkanRenderer::CreateViewportCommandPool()
{

}

void VulkanRenderer::CreateViewportFramebuffers()
{

}

void VulkanRenderer::CreateViewportCommandBuffers()
{

}

void VulkanRenderer::CreateViewportTextureSampler() 
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    CheckForError(vkCreateSampler(s_VulkanData.device, &samplerInfo, nullptr, &s_VulkanData.textureSampler) != VK_SUCCESS, "Failed to create texture sampler!")
}

void VulkanRenderer::ImGuiOnUpdate(uint32_t imageIndex)   
{
    ImGui_ImplVulkan_NewFrame(); 
    ImGui_ImplGlfw_NewFrame(); 
    ImGui::NewFrame(); 
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


    s_VulkanData.ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    s_VulkanData.ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    s_VulkanData.ubo.proj  = glm::perspective(glm::radians(45.0f), (float)s_VulkanData.swapChainExtent.width / (float)s_VulkanData.swapChainExtent.height, 0.1f, 10.0f);

    s_VulkanData.ubo.proj[1][1] *= -1;

    memcpy(s_VulkanData.uniformBuffersMapped[imageIndex], &s_VulkanData.ubo, sizeof(s_VulkanData.ubo));


    ImGui::Begin("Vulkan Renderer"); 



    ImGui::End();

    //VkDescriptorSet descriptorSet;   
    //descriptorSet = ImGui_ImplVulkan_AddTexture(s_VulkanData.textureSampler, s_VulkanData.viewportImageViews[imageIndex], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    ImGui::Begin("Viewport");
    //
    //ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    //ImGui::Image(descriptorSet, ImVec2{ viewportPanelSize.x, viewportPanelSize.y });
    
    ImGui::End(); 

    ImGui::ShowDemoWindow(); 
    ImGui::Render();  
}

void VulkanRenderer::ImGuiShutdown()
{
    // Resources to destroy on swapchain recreation
    for (auto framebuffer : s_VulkanData.imGuiFramebuffers)
        vkDestroyFramebuffer(s_VulkanData.device, framebuffer, nullptr);

    vkDestroyRenderPass(s_VulkanData.device, s_VulkanData.imGuiRenderPass, nullptr);
    vkDestroyCommandPool(s_VulkanData.device, s_VulkanData.imGuiCommandPool, nullptr);

    // Resources to destroy when the program ends
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(s_VulkanData.device, s_VulkanData.imGuiDescriptorPool, nullptr);
}
     
void recordImGuiCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(s_VulkanData.imGuiCommandBuffer, &info);

    VkImageMemoryBarrier imageBarrierImGui = {};
    imageBarrierImGui.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrierImGui.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageBarrierImGui.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierImGui.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  
    imageBarrierImGui.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  
    imageBarrierImGui.image = s_VulkanData.swapChainImages[imageIndex]; // Replace with the correct swapchain image 
    imageBarrierImGui.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrierImGui.subresourceRange.baseMipLevel = 0;
    imageBarrierImGui.subresourceRange.levelCount = 1;
    imageBarrierImGui.subresourceRange.baseArrayLayer = 0;
    imageBarrierImGui.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        s_VulkanData.imGuiCommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageBarrierImGui
    );

    VkClearValue clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    VkRenderPassBeginInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderInfo.renderPass = s_VulkanData.imGuiRenderPass;
    renderInfo.framebuffer = s_VulkanData.swapChainFramebuffers[imageIndex];
    renderInfo.renderArea.extent.width = s_VulkanData.swapChainExtent.width;
    renderInfo.renderArea.extent.height = s_VulkanData.swapChainExtent.height;
    renderInfo.clearValueCount = 1;
    renderInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(s_VulkanData.imGuiCommandBuffer, &renderInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), s_VulkanData.imGuiCommandBuffer);

    vkCmdEndRenderPass(s_VulkanData.imGuiCommandBuffer);
    vkEndCommandBuffer(s_VulkanData.imGuiCommandBuffer);
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
{                                                                                                                              
    // Initialize a VkCommandBufferBeginInfo structure
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr;

    // Begin recording a command buffer
    CheckForError(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS, "Failed to begin recording Command Buffer!");

    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = s_VulkanData.swapChainImages[imageIndex]; // Replace with the correct swapchain image
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        s_VulkanData.commandBuffers[currentFrame],
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageBarrier
    );

    // Initialize a VkRenderPassBeginInfo structure
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = s_VulkanData.renderPass;
    renderPassInfo.framebuffer = s_VulkanData.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = s_VulkanData.swapChainExtent;

    // Configure the clear color for the render pass
    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // Begin a render pass
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind a graphics pipeline to the command buffer
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_VulkanData.graphicsPipeline);

    // Set the viewport for the rendering
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(s_VulkanData.swapChainExtent.width);
    viewport.height = static_cast<float>(s_VulkanData.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // Set the scissor region for the rendering
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = s_VulkanData.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { s_VulkanData.vertexBuffer }; 
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);    
    vkCmdBindIndexBuffer(commandBuffer, s_VulkanData.indexBuffer, 0, VK_INDEX_TYPE_UINT16); 
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_VulkanData.pipelineLayout, 0, 1, &s_VulkanData.descriptorSets[currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);      

    // End the render pass
    vkCmdEndRenderPass(commandBuffer);

    // End recording the command buffer
    CheckForError(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS, "Failed to record Command Buffer!");
}
                                                                                                                                                                           
void drawFrame()                                                                                                                                                           
{                                                                                                                                                                          
    // Wait for the in-flight fence to signal, indicating the completion of previous frame's rendering
    vkWaitForFences(s_VulkanData.device, 1, &s_VulkanData.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire the next available image from the swap chain for rendering
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(s_VulkanData.device, s_VulkanData.swapChain, UINT64_MAX, s_VulkanData.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {      
        VulkanRenderer::RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        CheckForError(true, "Failed to acquire Swap Chain Image!");
    

    // Reset the command buffer for recording new commands
    vkResetCommandBuffer(s_VulkanData.commandBuffers[currentFrame], 0);
    recordCommandBuffer(s_VulkanData.commandBuffers[currentFrame], imageIndex); // Record rendering commands

    // ImGui
    if (s_VulkanData.EnableImGui) 
    {
        VulkanRenderer::ImGuiOnUpdate(imageIndex);
        vkResetCommandBuffer(s_VulkanData.imGuiCommandBuffer, 0);
        vkResetCommandPool(s_VulkanData.device, s_VulkanData.imGuiCommandPool, 0); 
        recordImGuiCommandBuffer(imageIndex); 
    }

    // Reset the in-flight fence for the next frame
    vkResetFences(s_VulkanData.device, 1, &s_VulkanData.inFlightFences[currentFrame]);

    // Configure the submission of rendering commands to the graphics queue
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { s_VulkanData.imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // ImGui Code
    if (s_VulkanData.EnableImGui)
    {
        std::array<VkCommandBuffer, 2> submitCommandBuffers =
        { s_VulkanData.commandBuffers[currentFrame], s_VulkanData.imGuiCommandBuffer };
        submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
        submitInfo.pCommandBuffers = submitCommandBuffers.data();
    }
    else
    {
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &s_VulkanData.commandBuffers[currentFrame];
    }
     
    VkSemaphore signalSemaphores[] = { s_VulkanData.renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Submit rendering commands to the graphics queue
    CheckForError(vkQueueSubmit(s_VulkanData.graphicsQueue, 1, &submitInfo, s_VulkanData.inFlightFences[currentFrame]) != VK_SUCCESS, "Failed to submit draw Command Buffer!");

    // Configure the presentation of the rendered image to the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { s_VulkanData.swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    // Present the rendered image to the screen
    result = vkQueuePresentKHR(s_VulkanData.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        auto windowData = GetWindow().GetData();
        windowData->framebufferResized = false;
        VulkanRenderer::RecreateSwapChain(); 
    }    
    else if (result != VK_SUCCESS) 
        CheckForError(true, "Failed to present Swap Chain Image!");
         
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;   
}    
    
void VulkanRenderer::OnUpdate()
{
    drawFrame();
    vkDeviceWaitIdle(s_VulkanData.device); 
}

void VulkanRenderer::RecreateSwapChain()    
{
    int width = 0, height = 0;
    auto window = GetWindow().GLFW();
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) 
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(s_VulkanData.device);

    CleanUpSwapChain(); 

    CreateSwapChain();  
    CreateImageViews();   
    CreateFramebuffers();   
}
     
void VulkanRenderer::CleanUpSwapChain() 
{
    for (size_t i = 0; i < s_VulkanData.swapChainFramebuffers.size(); i++)
        vkDestroyFramebuffer(s_VulkanData.device, s_VulkanData.swapChainFramebuffers[i], nullptr);
    
    for (size_t i = 0; i < s_VulkanData.swapChainImageViews.size(); i++)
        vkDestroyImageView(s_VulkanData.device, s_VulkanData.swapChainImageViews[i], nullptr);
    
    vkDestroySwapchainKHR(s_VulkanData.device, s_VulkanData.swapChain, nullptr);
}

void VulkanRenderer::Cleanup()
{   
    CleanUpSwapChain(); 
    vkDestroySampler(s_VulkanData.device, s_VulkanData.textureSampler, nullptr);
 
    for (size_t i = 0; i < s_VulkanData.swapChainImages.size(); i++)  
    {
        vkDestroyBuffer(s_VulkanData.device, s_VulkanData.uniformBuffers[i], nullptr);
        vkFreeMemory(s_VulkanData.device, s_VulkanData.uniformBuffersMemory[i], nullptr);
    }
  
    vkDestroyDescriptorPool(s_VulkanData.device, s_VulkanData.descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(s_VulkanData.device, s_VulkanData.descriptorSetLayout, nullptr);

    vkDestroyBuffer(s_VulkanData.device, s_VulkanData.indexBuffer, nullptr);
    vkFreeMemory(s_VulkanData.device, s_VulkanData.indexBufferMemory, nullptr);
   
    vkDestroyBuffer(s_VulkanData.device, s_VulkanData.vertexBuffer, nullptr);
    vkFreeMemory(s_VulkanData.device, s_VulkanData.vertexBufferMemory, nullptr);

    if (s_VulkanData.EnableImGui)
        ImGuiShutdown();

    for (size_t i = 0; i < s_VulkanData.swapChainImages.size(); i++)  
    {
        vkDestroySemaphore(s_VulkanData.device, s_VulkanData.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(s_VulkanData.device, s_VulkanData.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(s_VulkanData.device, s_VulkanData.inFlightFences[i], nullptr);
    }
    vkDestroyPipeline(s_VulkanData.device, s_VulkanData.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(s_VulkanData.device, s_VulkanData.pipelineLayout, nullptr);
    vkDestroyRenderPass(s_VulkanData.device, s_VulkanData.renderPass, nullptr);
 
    vkDestroyCommandPool(s_VulkanData.device, s_VulkanData.commandPool, nullptr);

    vkDestroySurfaceKHR(s_VulkanData.instance, s_VulkanData.surface, nullptr);
    vkDestroyDevice(s_VulkanData.device, nullptr);    

    if (enableValidationLayers)
        VulkanMessageCallback::DestroyDebugMessenger(s_VulkanData.instance);

    vkDestroyInstance(s_VulkanData.instance, nullptr);
}