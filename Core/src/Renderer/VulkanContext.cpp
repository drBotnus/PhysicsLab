#include "Renderer/VulkanContext.h"
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <string>
#include <set>
#include <algorithm>

namespace Core {

	void VulkanContext::Init(GLFWwindow* window, bool useValidation)
	{
		m_UseValidationLayers = useValidation;

		InitVulkan(window);
		InitSwapchain();
		InitCommands();
		InitSyncStructures();

		m_Initialized = true;
	}

	void VulkanContext::InitVulkan(GLFWwindow* window)
	{
		::vkb::InstanceBuilder builder;

		auto inst_ret = builder
			.set_app_name("PhysicsLab")
			.request_validation_layers(m_UseValidationLayers)
			.use_default_debug_messenger()
			.require_api_version(1, 4, 0)
			.build();

		::vkb::Instance vkb_inst = inst_ret.value();

		m_Instance = vkb_inst.instance;
		m_DebugMessenger = vkb_inst.debug_messenger;

		if (::glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}

		::VkPhysicalDeviceVulkan14Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES };
		
		::VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.synchronization2 = VK_TRUE;
		features13.dynamicRendering = VK_TRUE;

		::VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.bufferDeviceAddress = VK_TRUE;
		features12.descriptorIndexing = VK_TRUE;

		::vkb::PhysicalDeviceSelector selector{ vkb_inst };
		::vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 4)
			.set_required_features_14(features)
			.set_required_features_13(features13)
			.set_required_features_12(features12)
			.set_surface(m_Surface)
			.select()
			.value();

		::vkb::DeviceBuilder deviceBuilder{ physicalDevice };

		::vkb::Device vkbDevice = deviceBuilder.build().value();

		m_Device = vkbDevice.device;
		m_PhysicalDevice = physicalDevice.physical_device;

		m_GraphicsQueue = vkbDevice.get_queue(::vkb::QueueType::graphics).value();
		m_GraphicsQueueFamily = vkbDevice.get_queue_index(::vkb::QueueType::graphics).value();

		int width, height;

		glfwGetWindowSize(window, &width, &height);

		m_SwapchainExtent.width = static_cast<uint32_t>(width);
		m_SwapchainExtent.height = static_cast<uint32_t>(height);
	}

	void VulkanContext::CreateSwapchain(uint32_t width, uint32_t height) 
	{
		::vkb::SwapchainBuilder swapchainBuilder{ m_PhysicalDevice, m_Device, m_Surface };

		m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;

		::vkb::Swapchain vkbSwapchain = swapchainBuilder
			.set_desired_format(::VkSurfaceFormatKHR{ .format = m_SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(width, height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		m_SwapchainExtent = vkbSwapchain.extent;
		m_Swapchain = vkbSwapchain.swapchain;
		m_SwapchainImages = vkbSwapchain.get_images().value();
		m_PresentSemaphores.resize(m_SwapchainImages.size());

		m_SwapchainImageViews = vkbSwapchain.get_image_views().value();

		m_ImageFences.resize(m_SwapchainImages.size(), VK_NULL_HANDLE);
	}

	void VulkanContext::DestroySwapchain()
	{
		for (int i = 0; i < m_SwapchainImageViews.size(); i++) {
			::vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
		}

		::vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	}

	void VulkanContext::InitSwapchain()
	{
		CreateSwapchain(m_SwapchainExtent.width, m_SwapchainExtent.height);
	}

	void VulkanContext::InitCommands()
	{
		::VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_GraphicsQueueFamily;

		for (int i = 0; i < FRAME_OVERLAP; i++) {
			if (::vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_Frames[i].CommandPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create command pool!");
			}

			::VkCommandBufferAllocateInfo cmdAllocInfo = {};
			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.pNext = nullptr;
			cmdAllocInfo.commandPool = m_Frames[i].CommandPool;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			if (::vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_Frames[i].MainCommandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to allocate command buffers!");
			}
		}
	}

	void VulkanContext::InitSyncStructures()
	{
		::VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		::VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
			if (::vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].RenderFence) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create fence!");
			}

			if (::vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].SwapchainSemaphore) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create swapchain semaphore!");
			}
		}

		m_PresentSemaphores.resize(m_SwapchainImages.size());

		for (::VkSemaphore& semaphore : m_PresentSemaphores) {
			if (::vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create present semaphore!");
			}
		}
	}

	void VulkanContext::BeginFrame()
	{
		FrameData& frame = GetCurrentFrame();

		::vkWaitForFences(m_Device, 1, &frame.RenderFence, VK_TRUE, UINT64_MAX);

		::VkResult result = ::vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, frame.SwapchainSemaphore, VK_NULL_HANDLE, &m_SwapchainImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			// recreate swapchain later
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swapchain image!");
		}

		if (m_ImageFences[m_SwapchainImageIndex] != VK_NULL_HANDLE) {
			::vkWaitForFences(m_Device, 1, &m_ImageFences[m_SwapchainImageIndex], VK_TRUE, UINT64_MAX);
		}

		m_ImageFences[m_SwapchainImageIndex] = frame.RenderFence;

		::vkResetFences(m_Device, 1, &frame.RenderFence);

		::vkResetCommandBuffer(frame.MainCommandBuffer, 0);

		::VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (::vkBeginCommandBuffer(frame.MainCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin command buffer!");
		}

		m_FrameStarted = true;
	}

	void VulkanContext::TransitionImage(
		::VkCommandBuffer cmd,
		::VkImage image,
		::VkImageLayout oldLayout,
		::VkImageLayout newLayout)
	{
		::VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;

		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.image = image;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;

		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		::VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &barrier;

		::vkCmdPipelineBarrier2(cmd, &depInfo);
	}

	void VulkanContext::DrawBackground()
	{
		VkCommandBuffer cmd =
			GetCurrentCommandBuffer();

		VkImage image =
			m_SwapchainImages[m_SwapchainImageIndex];

		TransitionImage(
			cmd,
			image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkClearColorValue clearColor =
		{
			{ 0.0f, 0.0f, 1.0f, 1.0f }
		};

		VkImageSubresourceRange range{};
		range.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;

		range.baseMipLevel = 0;
		range.levelCount = 1;

		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vkCmdClearColorImage(
			cmd,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearColor,
			1,
			&range);

		TransitionImage(
			cmd,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void VulkanContext::EndFrame()
	{
		FrameData& frame = GetCurrentFrame();

		if (!m_FrameStarted)
			return;

		DrawBackground();

		if (::vkEndCommandBuffer(frame.MainCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}

		::VkCommandBufferSubmitInfo cmdInfo{};
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdInfo.commandBuffer = frame.MainCommandBuffer;

		::VkSemaphoreSubmitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = frame.SwapchainSemaphore;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;

		::VkSemaphore presentSemaphore = m_PresentSemaphores[m_SwapchainImageIndex];

		::VkSemaphoreSubmitInfo signalInfo{};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = presentSemaphore;
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

		::VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;

		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;

		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdInfo;

		if (::vkQueueSubmit2(m_GraphicsQueue, 1, &submitInfo, frame.RenderFence) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit command buffer");
		}

		::VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &presentSemaphore;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;

		presentInfo.pImageIndices = &m_SwapchainImageIndex;

		::VkResult result = ::vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			// Recreate swapchain
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swapchain image!");
		}

		m_FrameNumber++;
		m_FrameStarted = false;
	}

	void VulkanContext::Shutdown() 
	{
		if (m_Initialized) {
			::vkDeviceWaitIdle(m_Device);

			for (int i = 0; i < FRAME_OVERLAP; i++) {
				::vkDestroyCommandPool(m_Device, m_Frames[i].CommandPool, nullptr);

				::vkDestroyFence(m_Device, m_Frames[i].RenderFence, nullptr);
				::vkDestroySemaphore(m_Device, m_Frames[i].SwapchainSemaphore, nullptr);
			}

			for (::VkSemaphore semaphore : m_PresentSemaphores) {
				::vkDestroySemaphore(m_Device, semaphore, nullptr);
			}
			m_PresentSemaphores.clear();
			DestroySwapchain();
			::vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
			::vkDestroyDevice(m_Device, nullptr);
			::vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);			
			::vkDestroyInstance(m_Instance, nullptr);
		}
	}
}