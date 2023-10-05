#pragma once

#include <stdint.h> 

class VulkanRenderer
{
public:

	static void VulkanInit();
	static void CreateInstance();
	static void CreateSurface();
	static void PhysicalDevice();
	static void CreateLogicalDevice();
	static void CreateSwapChain();
	static void CreateImageViews();
	static void CreateRenderPass();

	static void CreateDescriptorSetLayout();
	static void CreateDescriptorPool();
	static void CreateDescriptorSets();

	static void CreateUniformBuffers();   

	static void CreateGraphicsPipeline(); 
	static void CreateFramebuffers();
	static void CreateCommandPool();
	static void CreateCommandBuffer();
	static void CreateSyncObjects();


	static void CreateVertexBuffer();
	static void CreateIndexBuffer(); 

	static void CreateViewportImages();
	static void CreateViewportImageViews(); 
	static void CreateViewportTextureSampler(); 
	static void CreateViewportRenderPass();  
	static void CreateViewportPipeline();  
	static void CreateViewportCommandPool();
	static void CreateViewportFramebuffers();
	static void CreateViewportCommandBuffers();

	static void UpdateUniformBuffer(uint32_t currentImage);
	static void OnUpdate(); 

	static void RecreateSwapChain();
	static void CleanUpSwapChain();
	static void Cleanup(); 
public:
	static void InitImGui();
	static void ImGuiOnUpdate(uint32_t imageIndex);
	static void ImGuiShutdown();
private:
	 
};


 