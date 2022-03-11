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
		explicit Buffer(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type);

		/**
		 * Destructor.
		 */
		~Buffer() override;

		/**
		 * Copy data from another buffer.
		 * This is needed because some buffer types does not allow mapping memory.
		 *
		 * @param pBuffer The buffer to copy data from.
		 */
		void fromBuffer(const Buffer* pBuffer) const;

		/**
		 * Bind the buffer to the command buffer as a vertex buffer.
		 * This is only possible if the type is Vertex.
		 *
		 * @param pCommandBuffer The command buffer pointer.
		 */
		void bindAsVertexBuffer(const CommandBuffer* pCommandBuffer);

		/**
		 * Bind the buffer to the command buffer as a index buffer.
		 * This is only possible if the type is Index.
		 *
		 * @param pCommandBuffer The command buffer pointer.
		 */
		void bindAsIndexBuffer(const CommandBuffer* pCommandBuffer, const VkIndexType indexType = VkIndexType::VK_INDEX_TYPE_UINT32);

		/**
		 * Terminate the buffer.
		 */
		void terminate() override;

		/**
		 * Map the buffer's memory to the local address space.
		 */
		std::byte* mapMemory();

		/**
		 * Unmap the mapped memory.
		 */
		void unmapMemory();

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
		static std::shared_ptr<Buffer> create(const std::shared_ptr<Engine>& pEngine, const uint64_t size, const BufferType type);

	private:
		const uint64_t m_Size = 0;
		VmaAllocation m_Allocation = nullptr;
		VkBuffer m_vBuffer = VK_NULL_HANDLE;
		const BufferType m_Type = BufferType::Unknown;
		VmaMemoryUsage m_MemoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
		bool m_bIsMapped = false;
	};
}