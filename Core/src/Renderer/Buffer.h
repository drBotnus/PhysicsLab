#pragma once

#include <vulkan/vulkan.h>

namespace Core {

class VulkanContext;

class Buffer
{
public:
	Buffer(::VkDevice device, ::VkPhysicalDevice physicalDevice);
	~Buffer();

	void Create(::VkDeviceSize size, ::VkBufferUsageFlags usage, ::VkMemoryPropertyFlags properties);

	void Destroy();

	void Upload(const void* data, size_t size);
	void Download(void* dst, size_t size);

	::VkBuffer GetHandle() const
	{
		return m_Buffer;
	}

private:
	uint32_t FindMemoryType(uint32_t typeFilter, ::VkMemoryPropertyFlags properties) const;

private:
	::VkDevice m_Device = VK_NULL_HANDLE;
	::VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

	::VkBuffer m_Buffer = VK_NULL_HANDLE;
	::VkDeviceMemory m_Memory = VK_NULL_HANDLE;

};

}