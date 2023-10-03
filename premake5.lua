include "Dependencies.lua"

workspace "Application"
   architecture "x86_64"
   startproject "Application"

	configurations
	{
		"Debug",
		"Release",
	}

project "Application"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   staticruntime "off"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

files
{
	"src/**.h",
   "src/**.cpp",
}

includedirs
{
   "src",
   "%{IncludeDir.VulkanSDK}",
   "%{IncludeDir.GLFW}",
   "%{IncludeDir.GLM}",
   "%{IncludeDir.Spdlog}",
   "%{IncludeDir.imgui}"
}

libdirs
{
   "%{LibraryDir.VulkanSDK}",
   "%{LibraryDir.GLFW}",
}

links 
{
   "%{Library.Vulkan}",
   "%{Library.VulkanUtils}",
   "%{Library.GLFW}",
}

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "RELEASE" }
      optimize "On"