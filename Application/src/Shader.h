#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class Shader
{
public:
	Shader();
	~Shader(); 

	static void Initialize(VkDevice device); 
	std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages(); 
private:
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages; 
};
