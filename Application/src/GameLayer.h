#pragma once
#include "Layer.h"

class GameLayer : public Layer
{
public:
	GameLayer();
	~GameLayer();

	void OnAttach();
	void OnUpdate();

private:
};