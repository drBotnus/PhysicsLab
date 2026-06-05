#pragma once
#include <string>
#include <memory>

#include "Core/Layer.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "Core/LayerStack.h"

namespace Core {

struct ApplicationSpecification
{
	std::string Name = "Core Engine";
	std::string WorkingDirectory;
};

class Application {
public:
    Application(const ApplicationSpecification& specification);
    virtual ~Application() = default;

    void Run();
    
	void PushLayer(Layer* layer);
private:
	bool m_Running = true;

	Window m_Window;
	std::unique_ptr<Renderer> m_Renderer;

	LayerStack m_LayerStack;
};

}