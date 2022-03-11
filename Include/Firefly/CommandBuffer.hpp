#pragma once

#include "EngineBoundObject.hpp"

namespace Firefly
{
	/**
	 * Command buffer object.
	 * Command buffers are used to submit commands to the GPU.
	 */
	class CommandBuffer final : public EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer to which this object is bound to.
		 * @param vCommandPool The command pool used to allocate this command buffer.
		 * @param vCommandBuffer The Vulkan command buffer.
		 */
		explicit CommandBuffer(const std::shared_ptr<Engine>& pEngine, const VkCommandPool vCommandPool, const VkCommandBuffer vCommandBuffer);

		/**
		 * Create a new command buffer.
		 *
		 * @param pEngine The engine pointer to which this object is bound to.
		 * @param vCommandPool The command pool used to allocate this command buffer.
		 * @param vCommandBuffer The Vulkan command buffer.
		 */
		static std::shared_ptr<CommandBuffer> create(const std::shared_ptr<Engine>& pEngine, const VkCommandPool vCommandPool, const VkCommandBuffer vCommandBuffer);

		/**
		 * Begin the command buffer recording.
		 */
		void begin();

		/**
		 * Bind a viewport to the command buffer.
		 * 
		 * @param viewport The viewport to bind.
		 */
		void bindViewport(const VkViewport viewport) const;

		/**
		 * Bind a scissor to the command buffer.
		 * 
		 * @param scissor The scissor to bind.
		 */
		void bindScissor(const VkRect2D scissor) const;

		/**
		 * Issue the draw vertices call.
		 *
		 * @param vertexCount The number of vertices to draw.
		 */
		void drawVertices(const uint32_t vertexCount) const;

		/**
		 * Issue the draw indices call.
		 *
		 * @param indexCount The number of indices to draw.
		 * @param vertexOffset The vertex offset to draw from. Default is 0.
		 */
		void drawIndices(const uint32_t indexCount, const uint32_t vertexOffset = 0) const;

		/**
		 * End command buffer recording.
		 */
		void end();

		/**
		 * Submit the recorded commands to the GPU.
		 *
		 * @param shouldWait Whether or not to wait till command buffer finishes execution. Default is true.
		 */
		void submit(bool shouldWait = true);

		/**
		 * Terminate the command buffer.
		 */
		void terminate() override;

		/**
		 * Get the command buffer.
		 *
		 * @return The command buffer.
		 */
		VkCommandBuffer getCommandBuffer() const { return m_vCommandBuffer; }

		/**
		 * Check if the command buffer is in the recording state.
		 *
		 * @return Boolean value stating if its recording or not.
		 */
		bool isRecording() const { return m_bIsRecording; }

		/**
		 * Get the in flight semaphore.
		 *
		 * @return The semaphore.
		 */
		VkSemaphore getInFlightSemaphore() const { return m_vInFlightSemaphore; }

		/**
		 * Get the render finished semaphore.
		 *
		 * @return The semaphore.
		 */
		VkSemaphore getRenderFinishedSemaphore() const { return m_vRenderFinishedSemaphore; }

	private:
		/**
		 * Create the semaphores.
		 */
		void createSemaphores();

	private:
		VkSemaphore m_vInFlightSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_vRenderFinishedSemaphore = VK_NULL_HANDLE;

		VkCommandPool m_vCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_vCommandBuffer = VK_NULL_HANDLE;

		bool m_bIsRecording = false;
	};
}