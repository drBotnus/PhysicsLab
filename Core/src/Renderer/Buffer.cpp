#include "Renderer/Buffer.h"
#include <stdexcept>
#include <cstring>

namespace Core {

Buffer::Buffer(::VkDevice device, ::VkPhysicalDevice physicalDevice)
    : m_Device(device), m_PhysicalDevice(physicalDevice)
{
}

Buffer::~Buffer()
{
    Destroy();
}

void Buffer::Create(::VkDeviceSize size, ::VkBufferUsageFlags usage, ::VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (::vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    ::VkMemoryRequirements memRequirements;
    ::vkGetBufferMemoryRequirements(
        m_Device,
        m_Buffer,
        &memRequirements);

    ::VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(
            memRequirements.memoryTypeBits,
            properties);

    if (::vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory!");
    }

    if (::vkBindBufferMemory(m_Device, m_Buffer, m_Memory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Failed to bind buffer memory!");
    }
}

void Buffer::Destroy()
{
    if (m_Buffer != VK_NULL_HANDLE)
    {
        ::vkDestroyBuffer(m_Device, m_Buffer, nullptr);
        m_Buffer = VK_NULL_HANDLE;
    }

    if (m_Memory != VK_NULL_HANDLE)
    {
        ::vkFreeMemory(m_Device, m_Memory, nullptr);
        m_Memory = VK_NULL_HANDLE;
    }
}

void Buffer::Upload(const void* data, size_t size)
{
    void* mapped;

    if (::vkMapMemory(m_Device, m_Memory, 0, size, 0, &mapped) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map memory!");
    }

    memcpy(mapped, data, size);

    ::vkUnmapMemory(m_Device, m_Memory);
}

void Buffer::Download(void* dst, size_t size)
{
    void* data;

    if (::vkMapMemory(m_Device, m_Memory, 0, size, 0, &data) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map memory!");
    }

    memcpy(dst, data, size);

    ::vkUnmapMemory(m_Device, m_Memory);
}

uint32_t Buffer::FindMemoryType(uint32_t typeFilter, ::VkMemoryPropertyFlags properties) const
{
    ::VkPhysicalDeviceMemoryProperties memProperties;
    ::vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

}