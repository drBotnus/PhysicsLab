#include "Core/Application.h"
#include "Core/EntryPoint.h"

class Sandbox : public Core::Application
{
public:
	Sandbox(const Core::ApplicationSpecification& specification) : Core::Application(specification)
	{
	}
};

Core::Application* CreateApplication()
{
	Core::ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../";

	return new Sandbox(spec);
}
