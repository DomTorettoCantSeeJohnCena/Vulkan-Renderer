#pragma once
#include <vector>
#include "Layer.h"

class LayerStack
{
public:
	LayerStack();
	~LayerStack();

	void PushLayer(Layer* layer);
	void LayerStack::PopLayer();
	void OnUpdate();


private:
	std::vector<Layer*> m_Layers;
};