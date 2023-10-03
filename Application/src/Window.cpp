#include "Window.h"
Window::Window(WindowProps props)
{
	windowData.w_TITLE = props.TITLE;
	windowData.w_WIDTH = props.WIDTH;
	windowData.w_HEIGHT = props.HEIGHT;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(windowData.w_WIDTH, windowData.w_HEIGHT, windowData.w_TITLE.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, &windowData);
	GLFWCallbackFunctionalities();
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::GLFWCallbackFunctionalities()
{
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.framebufferResized = true;	
	});
}

Window::WindowData* Window::GetData()
{
	return &windowData;
}
 
void Window::OnUpdate()
{
	glfwPollEvents();
}

void Window::Close(bool& running)
{
	if (glfwWindowShouldClose(window))
		running = false;
}