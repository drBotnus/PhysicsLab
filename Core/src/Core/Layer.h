#pragma once

namespace Core {
	
class Layer
{
public:
	virtual ~Layer() = default;

	virtual void OnAttach() {}
	virtual void OnUpdate(float dt) {}
	virtual void OnImGuiRender() {}
};

}