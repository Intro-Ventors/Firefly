#pragma once

#include "EngineBoundObject.hpp"

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
		 * @param memoryUsage The buffer's memory usage specification. If the default (UNKNOWN) is set, it will automatically select the best suitable for the type.
		 * @return The created buffer.
		 */
		explicit Buffer(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type, const VmaMemoryUsage memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN)
			: EngineBoundObject(pEngine), m_Size(size), m_Type(type), m_MemoryUsage(memoryUsage)
		{
			// Validate the inputs.
			if (size == 0)
				throw BackendError("Cannot create a buffer with 0 size!");

			else if (type == BufferType::Unknown)
				throw BackendError("The buffer type cannot be unknown!");

			// If the memory usage is not set, we automatically select the best for the given type.
			if (memoryUsage == VMA_MEMORY_USAGE_UNKNOWN)
			{
				switch (type)
				{
				case Firefly::BufferType::Vertex:
					m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_COPY;
					break;

				case Firefly::BufferType::Index:
					m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_COPY;
					break;

				case Firefly::BufferType::Uniform:
					m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
					break;

				case Firefly::BufferType::Staging:
					m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
					break;

				default:
					throw BackendError("Invalid buffer type!");
				}
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
		 * Terminate the buffer.
		 */
		void terminate() override
		{
			// Unmap if the buffer is mapped.
			if (m_bIsMapped)
				ummapMemory();

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
		void ummapMemory()
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
		 * Create a new buffer.
		 *
		 * @param pEngine The engine pointer.
		 * @param size The size of the buffer. Make sure that the buffer size is more than 0.
		 * @param type The buffer type.
		 * @param memoryUsage The buffer's memory usage specification. If the default (UNKNOWN) is set, it will automatically select the best suitable for the type.
		 * @return The created buffer.
		 */
		static std::shared_ptr<Buffer> create(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type, const VmaMemoryUsage memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN)
		{
			return std::make_shared<Buffer>(pEngine, size, type, memoryUsage);
		}

	private:
		uint64_t m_Size = 0;
		VmaAllocation m_Allocation = nullptr;
		VkBuffer m_vBuffer = VK_NULL_HANDLE;
		BufferType m_Type = BufferType::Unknown;
		VmaMemoryUsage m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
		bool m_bIsMapped = false;
	};
}