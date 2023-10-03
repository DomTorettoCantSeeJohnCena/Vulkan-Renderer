#include "Application.h"
#include "EntryPoint.h"
#include "GameLayer.h"

#include  <iostream>

Application* Application::s_Instance = nullptr;

Application::Application(CommandLineArguments arg)
	: m_CommandLineArguments(arg)
{
	if (s_Instance != nullptr)
		return;
			
	s_Instance = this;  

	m_Window = new Window();
	PushLayer(new GameLayer());
}

Application::~Application()
{
	delete m_Window;
	m_Window = nullptr;
}

void Application::PushLayer(Layer* layer)
{
	m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void Application::PopLayer()
{
	m_LayerStack.PopLayer();
}


Application& Application::GetApp()
{
	return *s_Instance;
}

Window& Application::GetWindow()
{
	return *m_Window;
}

void Application::Run()
{

	while (m_Running)
	{		
		m_Window->OnUpdate();
		m_Window->Close(m_Running);

		m_LayerStack.OnUpdate();	
	}

}