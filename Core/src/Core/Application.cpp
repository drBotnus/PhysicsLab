#include "Core/Application.h"

namespace Core {

Application::Application(const ApplicationSpecification& specification)
{
	m_Window.Init();
}

void Application::Run()
{
	while (m_Running)
	{
		m_Window.OnUpdate();

		if (m_Window.ShouldClose())
			m_Running = false;
	}
}

void Application::PushLayer(Layer* layer)
{
}

}