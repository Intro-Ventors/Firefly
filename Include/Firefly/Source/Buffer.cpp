#include "Firefly/Buffer.hpp"

namespace Firefly
{
	Buffer::Buffer(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type)
		: EngineBoundObject(pEngine), m_Size(size), m_Type(type)
	{
		// Validate the inputs.
		if (size == 0)
			throw BackendError("Cannot create a buffer with 0 size!");

		else if (type == BufferType::Unknown)
			throw BackendError("The buffer type cannot be unknown!");

		VmaAllocationCreateFlags vmaFlags = 0;

		// If the memory usage is not set, we automatically select the best for the given type.
		switch (type)
		{
		case Firefly::BufferType::Vertex:
		case Firefly::BufferType::Index:
			m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;

		case Firefly::BufferType::Uniform:
		case Firefly::BufferType::Staging:
			m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			vmaFlags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			break;

		default:
			throw BackendError("Invalid buffer type!");
		}

		// Create the buffer.
		VkBufferCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vCreateInfo.pNext = nullptr;
		vCreateInfo.flags = 0;
		vCreateInfo.size = m_Size;
		vCreateInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		vCreateInfo.queueFamilyIndexCount = 0;
		vCreateInfo.pQueueFamilyIndices = nullptr;
		vCreateInfo.usage = static_cast<VkBufferUsageFlags>(type);

		VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
		vmaAllocationCreateInfo.usage = m_MemoryUsage;
		vmaAllocationCreateInfo.flags = vmaFlags;

		FIREFLY_VALIDATE(vmaCreateBuffer(getEngine()->getAllocator(), &vCreateInfo, &vmaAllocationCreateInfo, &m_vBuffer, &m_Allocation, nullptr), "Failed to create the buffer!");
	}

	Buffer::~Buffer()
	{
		if (!isTerminated())
			terminate();
	}
	
	void Buffer::fromBuffer(const Buffer* pBuffer) const
	{
		// If the incoming buffer is nullptr, lets just skip.
		if (!pBuffer)
			return;

		// Validate the incoming buffer size.
		if (pBuffer->size() > m_Size)
			throw BackendError("The source buffer size is larger than what's available!");

		// Setup copy info.
		VkBufferCopy vCopy = {};
		vCopy.srcOffset = 0;
		vCopy.dstOffset = 0;
		vCopy.size = pBuffer->size();

		// Copy the buffer.
		const auto vCommandBuffer = getEngine()->beginCommandBufferRecording();
		getEngine()->getDeviceTable().vkCmdCopyBuffer(vCommandBuffer, pBuffer->getBuffer(), m_vBuffer, 1, &vCopy);
		getEngine()->executeRecordedCommands();
	}
	
	void Buffer::bindAsVertexBuffer(const CommandBuffer* pCommandBuffer)
	{
		// Validate the buffer type.
		if (m_Type != BufferType::Vertex)
			throw BackendError("Cannot bind the buffer as a Vertex buffer! The types does not match.");

		// Now we can bind it.
		std::array<VkDeviceSize, 1> offset = { 0 };
		getEngine()->getDeviceTable().vkCmdBindVertexBuffers(pCommandBuffer->getCommandBuffer(), 0, 1, &m_vBuffer, offset.data());
	}
	
	void Buffer::bindAsIndexBuffer(const CommandBuffer* pCommandBuffer, const VkIndexType indexType)
	{
		// Validate the buffer type.
		if (m_Type != BufferType::Index)
			throw BackendError("Cannot bind the buffer as a Index buffer! The types does not match.");

		// Now we can bind it.
		getEngine()->getDeviceTable().vkCmdBindIndexBuffer(pCommandBuffer->getCommandBuffer(), m_vBuffer, 0, indexType);
	}
	
	void Buffer::terminate()
	{
		// Unmap if the buffer is mapped.
		if (m_bIsMapped)
			unmapMemory();

		vmaDestroyBuffer(getEngine()->getAllocator(), m_vBuffer, m_Allocation);
		toggleTerminated();
	}
	
	std::byte* Buffer::mapMemory()
	{
		std::byte* pDataPointer = nullptr;
		FIREFLY_VALIDATE(vmaMapMemory(getEngine()->getAllocator(), m_Allocation, reinterpret_cast<void**>(&pDataPointer)), "Failed to map the buffer memory!");

		m_bIsMapped = true;
		return pDataPointer;
	}
	
	void Buffer::unmapMemory()
	{
		// We only need to unmap if we have mapped the memory.
		if (m_bIsMapped)
		{
			vmaUnmapMemory(getEngine()->getAllocator(), m_Allocation);
			m_bIsMapped = false;
		}
	}
	
	std::shared_ptr<Buffer> Buffer::create(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type)
	{
		return std::make_shared<Buffer>(pEngine, size, type);
	}
}