#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <optional>

namespace Core {

struct FrameData 
{
	::VkCommandPool CommandPool;
	::VkCommandBuffer MainCommandBuffer;

	::VkSemaphore SwapchainSemaphore;
	::VkFence RenderFence;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanContext
{
public:
	void Init(GLFWwindow* window, bool useValidation);

	void BeginFrame();
	void EndFrame();

	void Shutdown();

	bool IsInitialized() { return m_Initialized; }

	FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; }

	::VkImage GetCurrentSwapchainImage() { return m_SwapchainImages[m_SwapchainImageIndex]; }
	::VkImageView GetCurrentSwapchainImageView() { return m_SwapchainImageViews[m_SwapchainImageIndex]; }

	uint32_t GetFrameNumber() const
	{
		return m_FrameNumber;
	}

	::VkDevice GetDevice() const
	{
		return m_Device;
	}

	::VkPhysicalDevice GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	::VkQueue GetGraphicsQueue() const
	{
		return m_GraphicsQueue;
	}

	::VkCommandBuffer GetCurrentCommandBuffer()
	{
		return GetCurrentFrame().MainCommandBuffer;
	}

private:
	void InitVulkan(GLFWwindow* window);
	void InitSwapchain();
	void InitCommands();
	void InitSyncStructures();

	void TransitionImage(::VkCommandBuffer cmd, ::VkImage image, ::VkImageLayout oldLayout, ::VkImageLayout newLayout);
	void DrawBackground();
	
	void CreateSwapchain(uint32_t width, uint32_t height);

	void DestroySwapchain();

private:
	::VkInstance m_Instance = VK_NULL_HANDLE;
	::VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
	::VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	::VkDevice m_Device = VK_NULL_HANDLE;
	::VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	::VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	::VkFormat m_SwapchainImageFormat;

	std::vector<::VkImage> m_SwapchainImages;
	std::vector<::VkImageView> m_SwapchainImageViews;
	std::vector<::VkSemaphore> m_PresentSemaphores;
	std::vector<::VkFence> m_ImageFences;

	::VkExtent2D m_SwapchainExtent;

	FrameData m_Frames[FRAME_OVERLAP];
	
	::VkQueue m_GraphicsQueue;
	uint32_t m_GraphicsQueueFamily;

	int m_FrameNumber = 0;
	uint32_t m_SwapchainImageIndex = 0;

	bool m_FrameStarted = false;

	bool m_Initialized = false;
	bool m_UseValidationLayers = false;
};

}