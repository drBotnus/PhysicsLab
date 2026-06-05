#pragma once

#include "Core/Layer.h"

#include <vector>

namespace Core {

class LayerStack
{
public:
	void PushLayer(Layer* layer);
	void PopLayer(Layer* layer);

	std::vector<Layer*>::iterator begin();
	std::vector<Layer*>::iterator end();

private:
	std::vector<Layer*> m_Layers;
};

}