#pragma once

#include <memory>

#include "Renderer/VulkanContext.h"
#include "Platform/Window.h"

namespace Core {

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Init(Window& window);
	void Shutdown();

	void BeginFrame();
	void EndFrame();

	VulkanContext& GetContext() { return *m_Context; }

private:
	std::unique_ptr<VulkanContext> m_Context;
};

}