#pragma once


class Layer
{
public:
    Layer();
	virtual ~Layer();

	virtual void OnAttach() = 0;
	virtual void OnUpdate() = 0;

private:
};
