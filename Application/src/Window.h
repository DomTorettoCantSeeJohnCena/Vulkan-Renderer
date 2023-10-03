#pragma once

#include "GLFW/glfw3.h"
#include <string>

struct WindowProps
{
	std::string TITLE;
	uint32_t WIDTH;
	uint32_t HEIGHT;

	WindowProps(const std::string& title = "Vulkan Application",
		uint32_t width = 1600,
		uint32_t height = 900)
		: TITLE(title), WIDTH(width), HEIGHT(height)
	{
	}
};


class Window
{
public:
	Window(WindowProps props = WindowProps());
	~Window();

	void GLFWCallbackFunctionalities();

	void OnUpdate();

	GLFWwindow* GLFW() { return window; }

	void Close(bool& running);
private:

	struct WindowData
	{
		std::string w_TITLE;
		uint32_t w_WIDTH;
		uint32_t w_HEIGHT;
		bool framebufferResized = false;
	};

public:
	WindowData* GetData();
private:
	WindowData windowData;
	GLFWwindow* window;
};