#pragma once

#include "Window.h"
#include "LayerStack.h"

struct CommandLineArguments
{
	CommandLineArguments(int argc, char** argv)
		: m_Argc(argc), m_buffer(argv)
	{	
	}

	int GetArgumentCount() const
	{

	}

	char* GetData(int i) const
	{
		char* buffer = m_buffer[i];
		return buffer;
	}

	int GetSize() const
	{
		return sizeof(m_buffer);
	}

	int m_Argc;
	char** m_buffer = nullptr;
};

class Application
{
public:
	Application(CommandLineArguments arg);
	~Application();

	static Application& GetApp();
	Window& GetWindow();

	void PushLayer(Layer* layer);
	void PopLayer();

    void Run();


private:
	bool m_Running = true;
	static Application* s_Instance;

	CommandLineArguments m_CommandLineArguments;
	Window* m_Window = nullptr;
	LayerStack m_LayerStack;
};

Window& GetWindow()
{
	return Application::GetApp().GetWindow();
}

Application* CreateApplication(int argc, char** argv)
{
	CommandLineArguments c_arg(argc, argv);

	return new Application(c_arg);
}