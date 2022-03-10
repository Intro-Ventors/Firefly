#pragma once

#include "EngineBoundObject.hpp"

namespace Firefly
{
	/**
	 * Command buffer object.
	 * Command buffers are used to submit commands to the GPU.
	 */
	class CommandBuffer : public EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer to which this object is bound to.
		 * @param vCommandPool The command pool used to allocate this command buffer.
		 * @param vCommandBuffer The Vulkan command buffer.
		 */
		explicit CommandBuffer(const std::shared_ptr<Engine>& pEngine, const VkCommandPool vCommandPool, const VkCommandBuffer vCommandBuffer)
			: EngineBoundObject(pEngine), m_vCommandPool(vCommandPool), m_vCommandBuffer(vCommandBuffer)
		{
			// Create the semaphores.
			createSemaphores();
		}

		/**
		 * Create a new command buffer.
		 *
		 * @param pEngine The engine pointer to which this object is bound to.
		 * @param vCommandPool The command pool used to allocate this command buffer.
		 * @param vCommandBuffer The Vulkan command buffer.
		 */
		static std::shared_ptr<CommandBuffer> create(const std::shared_ptr<Engine>& pEngine, const VkCommandPool vCommandPool, const VkCommandBuffer vCommandBuffer)
		{
			return std::make_shared<CommandBuffer>(pEngine, vCommandPool, vCommandBuffer);
		}

		/**
		 * Begin the command buffer recording.
		 */
		void begin()
		{
			// If its in the recording state before this call, lets end it.
			if (isRecording())
				end();

			// Create the begin info structure.
			VkCommandBufferBeginInfo vBeginInfo = {};
			vBeginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			vBeginInfo.pNext = nullptr;
			vBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkBeginCommandBuffer(m_vCommandBuffer, &vBeginInfo), "Failed to begin command buffer recording!");
			m_bIsRecording = true;
		}

		/**
		 * Bind a viewport to the command buffer.
		 * 
		 * @param viewport The viewport to bind.
		 */
		void bindViewport(const VkViewport viewport) const
		{
			getEngine()->getDeviceTable().vkCmdSetViewport(m_vCommandBuffer, 0, 1, &viewport);
		}

		/**
		 * Bind a scissor to the command buffer.
		 * 
		 * @param scissor The scissor to bind.
		 */
		void bindScissor(const VkRect2D scissor) const
		{
			getEngine()->getDeviceTable().vkCmdSetScissor(m_vCommandBuffer, 0, 1, &scissor);
		}

		/**
		 * Issue the draw vertices call.
		 *
		 * @param vertexCount The number of vertices to draw.
		 */
		void drawVertices(const uint32_t vertexCount) const
		{
			getEngine()->getDeviceTable().vkCmdDraw(m_vCommandBuffer, vertexCount, 1, 0, 0);
		}

		/**
		 * Issue the draw indices call.
		 *
		 * @param indexCount The number of indices to draw.
		 * @param vertexOffset The vertex offset to draw from. Default is 0.
		 */
		void drawIndices(const uint32_t indexCount, const uint32_t vertexOffset = 0) const
		{
			getEngine()->getDeviceTable().vkCmdDrawIndexed(m_vCommandBuffer, indexCount, 1, 0, vertexOffset, 0);
		}

		/**
		 * End command buffer recording.
		 */
		void end()
		{
			// Just return if we are not recording.
			if (!isRecording())
				return;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkEndCommandBuffer(m_vCommandBuffer), "Failed to end command buffer recording!");
			m_bIsRecording = false;
		}

		/**
		 * Submit the recorded commands to the GPU.
		 *
		 * @param shouldWait Whether or not to wait till command buffer finishes execution. Default is true.
		 */
		void submit(bool shouldWait = true)
		{
			// End recording if we haven't.
			end();

			const auto queue = getEngine()->getQueue(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);

			VkPipelineStageFlags vWaitStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			// Create the submit info structure.
			VkSubmitInfo vSubmitInfo = {};
			vSubmitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
			vSubmitInfo.commandBufferCount = 1;
			vSubmitInfo.pCommandBuffers = &m_vCommandBuffer;
			vSubmitInfo.pWaitDstStageMask = &vWaitStageMask;
			//vSubmitInfo.signalSemaphoreCount = 1;
			//vSubmitInfo.pSignalSemaphores = &m_vRenderFinishedSemaphore;
			//vSubmitInfo.waitSemaphoreCount = 1;
			//vSubmitInfo.pWaitSemaphores = &m_vInFlightSemaphore;

			VkFence vFence = VK_NULL_HANDLE;

			// Create the fence if we need to wait.
			if (shouldWait)
			{
				VkFenceCreateInfo vFenceCreateInfo = {};
				vFenceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				vFenceCreateInfo.pNext = VK_NULL_HANDLE;
				vFenceCreateInfo.flags = 0;

				Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateFence(getEngine()->getLogicalDevice(), &vFenceCreateInfo, nullptr, &vFence), "Failed to create the synchronization fence!");
			}

			// Submit the queue.
			Utility::ValidateResult(getEngine()->getDeviceTable().vkQueueSubmit(queue.getQueue(), 1, &vSubmitInfo, vFence), "Failed to submit the queue!");

			// Destroy the fence if we created it.
			if (shouldWait)
			{
				Utility::ValidateResult(getEngine()->getDeviceTable().vkWaitForFences(getEngine()->getLogicalDevice(), 1, &vFence, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait for the fence!");
				getEngine()->getDeviceTable().vkDestroyFence(getEngine()->getLogicalDevice(), vFence, nullptr);
			}
		}

		/**
		 * Terminate the command buffer.
		 */
		void terminate() override
		{
			getEngine()->getDeviceTable().vkFreeCommandBuffers(getEngine()->getLogicalDevice(), m_vCommandPool, 1, &m_vCommandBuffer);
			getEngine()->getDeviceTable().vkDestroySemaphore(getEngine()->getLogicalDevice(), m_vInFlightSemaphore, nullptr);
			getEngine()->getDeviceTable().vkDestroySemaphore(getEngine()->getLogicalDevice(), m_vRenderFinishedSemaphore, nullptr);
			toggleTerminated();
		}

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
		void createSemaphores()
		{
			// Create the semaphore create info structure.
			VkSemaphoreCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.flags = 0;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateSemaphore(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vInFlightSemaphore), "Failed to create the in flight semaphore!");
			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateSemaphore(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vRenderFinishedSemaphore), "Failed to create the render finished semaphore!");
		}

	private:
		VkSemaphore m_vInFlightSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_vRenderFinishedSemaphore = VK_NULL_HANDLE;

		VkCommandPool m_vCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_vCommandBuffer = VK_NULL_HANDLE;

		bool m_bIsRecording = false;
	};
}