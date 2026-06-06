#include "Renderer/Renderer.h"

namespace Core {

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Init(Window& window)
{
	m_Context = std::make_unique<VulkanContext>();
	m_Context->Init(window.GetGLFWWindow(), true);
}

void Renderer::Shutdown()
{
	if (m_Context) {
		m_Context->Shutdown();
		m_Context.reset();
	}
}

void Renderer::BeginFrame()
{
	m_Context->BeginFrame();
}

void Renderer::EndFrame()
{
	m_Context->EndFrame();
}

}