#pragma once

#include "EngineBoundObject.hpp"
#include "CommandBuffer.hpp"

namespace Firefly
{
	/**
	 * Buffer type enum.
	 * This indicates the buffer type when creating a buffer.
	 */
	enum class BufferType : uint32_t
	{
		Unknown = 0,
		Vertex = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		Index = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		Uniform = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		Staging = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT
	};

	/**
	 * Buffer class.
	 * This object is used to store data in a GPU buffer.
	 */
	class Buffer final : public EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 * @param size The size of the buffer. Make sure that the buffer size is more than 0.
		 * @param type The buffer type.
		 * @return The created buffer.
		 */
		explicit Buffer(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type)
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

			Utility::ValidateResult(vmaCreateBuffer(getEngine()->getAllocator(), &vCreateInfo, &vmaAllocationCreateInfo, &m_vBuffer, &m_Allocation, nullptr), "Failed to create the buffer!");
		}

		/**
		 * Destructor.
		 */
		~Buffer() override
		{
			if (!isTerminated())
				terminate();
		}

		/**
		 * Copy data from another buffer.
		 * This is needed because some buffer types does not allow mapping memory.
		 *
		 * @param pBuffer The buffer to copy data from.
		 */
		void fromBuffer(const Buffer* pBuffer) const
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

		/**
		 * Bind the buffer to the command buffer as a vertex buffer.
		 * This is only possible if the type is Vertex.
		 *
		 * @param pCommandBuffer The command buffer pointer.
		 */
		void bindAsVertexBuffer(const CommandBuffer* pCommandBuffer)
		{
			// Validate the buffer type.
			if (m_Type != BufferType::Vertex)
				throw BackendError("Cannot bind the buffer as a Vertex buffer! The types does not match.");

			// Now we can bind it.
			std::array<VkDeviceSize, 1> offset = { 0 };
			getEngine()->getDeviceTable().vkCmdBindVertexBuffers(pCommandBuffer->getCommandBuffer(), 0, 1, &m_vBuffer, offset.data());
		}

		/**
		 * Bind the buffer to the command buffer as a index buffer.
		 * This is only possible if the type is Index.
		 *
		 * @param pCommandBuffer The command buffer pointer.
		 */
		void bindAsIndexBuffer(const CommandBuffer* pCommandBuffer, const VkIndexType indexType = VkIndexType::VK_INDEX_TYPE_UINT32)
		{
			// Validate the buffer type.
			if (m_Type != BufferType::Index)
				throw BackendError("Cannot bind the buffer as a Index buffer! The types does not match.");

			// Now we can bind it.
			getEngine()->getDeviceTable().vkCmdBindIndexBuffer(pCommandBuffer->getCommandBuffer(), m_vBuffer, 0, indexType);
		}

		/**
		 * Terminate the buffer.
		 */
		void terminate() override
		{
			// Unmap if the buffer is mapped.
			if (m_bIsMapped)
				unmapMemory();

			vmaDestroyBuffer(getEngine()->getAllocator(), m_vBuffer, m_Allocation);
			toggleTerminated();
		}

		/**
		 * Map the buffer's memory to the local address space.
		 */
		std::byte* mapMemory()
		{
			std::byte* pDataPointer = nullptr;
			Utility::ValidateResult(vmaMapMemory(getEngine()->getAllocator(), m_Allocation, reinterpret_cast<void**>(&pDataPointer)), "Failed to map the buffer memory!");

			m_bIsMapped = true;
			return pDataPointer;
		}

		/**
		 * Unmap the mapped memory.
		 */
		void unmapMemory()
		{
			// We only need to unmap if we have mapped the memory.
			if (m_bIsMapped)
			{
				vmaUnmapMemory(getEngine()->getAllocator(), m_Allocation);
				m_bIsMapped = false;
			}
		}

		/**
		 * Get the buffer size.
		 *
		 * @retrun The buffer size.
		 */
		uint64_t size() const { return m_Size; }

		/**
		 * Get the memory usage.
		 *
		 * @return The memory usage setting.
		 */
		VmaMemoryUsage getMemoryUsage() const { return m_MemoryUsage; }

		/**
		 * Get the Vulkan buffer.
		 *
		 * @return The buffer.
		 */
		VkBuffer getBuffer() const { return m_vBuffer; }

		/**
		 * Create a new buffer.
		 *
		 * @param pEngine The engine pointer.
		 * @param size The size of the buffer. Make sure that the buffer size is more than 0.
		 * @param type The buffer type.
		 * @return The created buffer.
		 */
		static std::shared_ptr<Buffer> create(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type)
		{
			return std::make_shared<Buffer>(pEngine, size, type);
		}

	private:
		const uint64_t m_Size = 0;
		VmaAllocation m_Allocation = nullptr;
		VkBuffer m_vBuffer = VK_NULL_HANDLE;
		const BufferType m_Type = BufferType::Unknown;
		VmaMemoryUsage m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
		bool m_bIsMapped = false;
	};
}