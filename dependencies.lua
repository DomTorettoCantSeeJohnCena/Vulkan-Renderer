IncludeDir = {}
IncludeDir["VulkanSDK"] = "C:/VulkanSDK/1.3.224.1/Include"
IncludeDir["GLFW"] =  "%{wks.location}/Application/Vendor/GLFW/include"
IncludeDir["GLM"] = "%{wks.location}/Application/Vendor/GLM"
IncludeDir["Spdlog"] = "%{wks.location}/Application/Vendor/Spdlog/include"
--IncludeDir["STBImage"] = "%{wks.location}/Application/Vendor/stb"
IncludeDir["imgui"] = "%{wks.location}/Application/Vendor/imgui"
--IncludeDir["tol"] = "%{wks.location}/Application/Vendor/tinyobjloader"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "C:/VulkanSDK/1.3.224.1/Lib"
LibraryDir["GLFW"] = "%{wks.location}/Application/Vendor/GLFW/lib-vc2022"

Library = {}
Library["Vulkan"] = "vulkan-1.lib"
Library["VulkanUtils"] = "VkLayer_utils.lib"
Library["GLFW"] = "glfw3.lib"