#include "Core/Application.h"
#include "Renderer/Buffer.h"
#include <iostream>

namespace Core {

Application::Application(const ApplicationSpecification& specification)
{
	m_Window.Init();

	m_Renderer = std::make_unique<Renderer>();
	m_Renderer->Init(m_Window);
}

void Application::Run()
{
	while (m_Running)
	{
		m_Window.OnUpdate();

		if (m_Window.ShouldClose())
			m_Running = false;

		m_Renderer->BeginFrame();
		m_Renderer->EndFrame();
	}

	m_Renderer->Shutdown();
	m_Window.Shutdown();
}

void Application::PushLayer(Layer* layer)
{
}

}