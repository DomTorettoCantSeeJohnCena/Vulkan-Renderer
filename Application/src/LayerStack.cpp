#include "LayerStack.h"

LayerStack::LayerStack()
{

}

LayerStack::~LayerStack()
{
	for (auto layer : m_Layers)
	{
		delete layer;
		layer = nullptr;
	}

	m_Layers.clear();
}

void LayerStack::PushLayer(Layer* layer)
{
	m_Layers.emplace_back(layer);
}

void LayerStack::PopLayer()
{
	m_Layers.pop_back();
}

void LayerStack::OnUpdate()
{
	for (int i = 0; i < m_Layers.size(); i++)
	{
		m_Layers[i]->OnUpdate();
	}
}