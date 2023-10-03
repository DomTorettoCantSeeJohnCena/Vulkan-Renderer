#pragma once

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
	static void CreateGraphicsPipeline(); 
	static void CreateFramebuffers();
	static void CreateCommandPool();
	static void CreateCommandBuffer();
	static void CreateSyncObjects();


	static void CreateVertexBuffer(); 

	static void CreateViewportImage();
	static void CreateViewportImageViews();


	static void OnUpdate(); 

	static void RecreateSwapChain();
	static void CleanUpSwapChain();
	static void Cleanup(); 
public:
	static void InitImGui();
	static void ImGuiOnUpdate();
	static void ImGuiShutdown();
private:
	
};


