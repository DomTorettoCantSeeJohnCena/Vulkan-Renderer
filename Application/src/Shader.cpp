#include "Shader.h"

#include <iostream> 
#include <fstream>
#include <map>

namespace Utils
{
    static std::vector<char> readFile(const std::string& filename)
    {
        // Open the file in binary mode and set the file pointer to the end
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        // Check if the file failed to open
        if (!file.is_open())
        {
            std::cout << "Failed to open file!" << std::endl;

            // Trigger a debugger breakpoint for debugging purposes
            __debugbreak();
        }

        // Get the size of the file by retrieving the current position of the file pointer
        size_t fileSize = (size_t)file.tellg();

        // Create a buffer to hold the file contents with the size of the file
        std::vector<char> buffer(fileSize);

        // Move the file pointer back to the beginning of the file
        file.seekg(0);

        // Read the entire file into the buffer
        file.read(buffer.data(), fileSize);

        // Close the file
        file.close();

        // Return the vector containing the file contents
        return buffer;
    }
}

struct ShaderData
{
    VkDevice device;
};

ShaderData* s_ShaderData = new ShaderData();

Shader::Shader()
{
    // Read vertex and fragment shader code from respective files
    auto vertShaderCode = Utils::readFile("Application/Shaders/vert.spv");
    auto fragShaderCode = Utils::readFile("Application/Shaders/frag.spv");

    // Create shader module info for the vertex shader
    VkShaderModuleCreateInfo vertCreateInfo{};
    vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertCreateInfo.codeSize = vertShaderCode.size();
    vertCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());

    // Create shader module info for the fragment shader
    VkShaderModuleCreateInfo fragCreateInfo{};
    fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragCreateInfo.codeSize = fragShaderCode.size();
    fragCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());

    // Create vertex shader module using the device
    vkCreateShaderModule(s_ShaderData->device, &vertCreateInfo, nullptr, &vertShaderModule); 
    // Create fragment shader module using the device
    vkCreateShaderModule(s_ShaderData->device, &fragCreateInfo, nullptr, &fragShaderModule); 

    // Create vertex shader stage info
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{}; 
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // Entry point function name in the shader code

    // Create fragment shader stage info
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{}; 
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; // Entry point function name in the shader code

    // Combine shader stage infos into an vector for the pipeline creation
    shaderStages.push_back(vertShaderStageInfo); 
    shaderStages.push_back(fragShaderStageInfo); 
}

Shader::~Shader()
{
    vkDestroyShaderModule(s_ShaderData->device, fragShaderModule, nullptr);
    vkDestroyShaderModule(s_ShaderData->device, vertShaderModule, nullptr);
    delete s_ShaderData;
    s_ShaderData = nullptr;
}

void Shader::Initialize(VkDevice device)
{
    s_ShaderData->device = device;
}

std::vector<VkPipelineShaderStageCreateInfo>& Shader::GetShaderStages()
{
    return shaderStages; 
}