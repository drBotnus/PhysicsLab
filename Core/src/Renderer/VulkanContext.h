#pragma once

namespace Core {

class VulkanContext
{
public:
	void Init();
	void Shutdown();

private:
	void CreateInstance();
	void PickPhysicalDevice();
	void CreateDevice();
};

}