#pragma once

#include "Application.h"

int main(int argc, char** argv)
{
	Application* app = CreateApplication(argc, argv);

	app->Run();

	delete app;

}