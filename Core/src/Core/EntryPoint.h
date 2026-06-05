#pragma once

#include "Core/Application.h"

extern Core::Application* CreateApplication();

int main()
{
	Core::Application* app = CreateApplication();
	app->Run();
	delete app;
}