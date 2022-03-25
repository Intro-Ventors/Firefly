#include "Firefly/CommandBuffer.hpp"
#include "Firefly/Graphics/RenderTarget.hpp"
#include "Firefly/Graphics/GraphicsPipeline.hpp"

#include <array>

#ifdef max
#undef max

#endif

namespace Firefly
{
	CommandBuffer::CommandBuffer(const std::shared_ptr<Engine>& pEngine, const VkCommandPool vCommandPool, const VkCommandBuffer vCommandBuffer)
		: EngineBoundObject(pEngine), m_vCommandPool(vCommandPool), m_vCommandBuffer(vCommandBuffer)
	{
		initialize();
	}

	std::shared_ptr<CommandBuffer> CommandBuffer::create(const std::shared_ptr<Engine>& pEngine, const VkCommandPool vCommandPool, const VkCommandBuffer vCommandBuffer)
	{
		const auto pointer = std::make_shared<CommandBuffer>(pEngine, vCommandPool, vCommandBuffer);
		FIREFLY_VALIDATE_OBJECT(pointer);

		return pointer;
	}

	void CommandBuffer::begin()
	{
		// If its in the recording state before this call, lets end it.
		if (isRecording())
			end();

		// Create the begin info structure.
		VkCommandBufferBeginInfo vBeginInfo = {};
		vBeginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vBeginInfo.pNext = nullptr;
		vBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkBeginCommandBuffer(m_vCommandBuffer, &vBeginInfo), "Failed to begin command buffer recording!");
		m_bIsRecording = true;
	}

	void CommandBuffer::bindRenderTarget(const RenderTarget* pRenderTarget, const std::vector<VkClearValue>& vClearColors) const
	{
		// Create the begin info structure.
		VkRenderPassBeginInfo vBeginInfo = {};
		vBeginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vBeginInfo.pNext = VK_NULL_HANDLE;
		vBeginInfo.renderPass = pRenderTarget->getRenderPass();
		vBeginInfo.framebuffer = pRenderTarget->getCurrentFrameBuffer();
		vBeginInfo.clearValueCount = static_cast<uint32_t>(vClearColors.size());
		vBeginInfo.pClearValues = vClearColors.data();
		vBeginInfo.renderArea.extent.width = pRenderTarget->getExtent().width;
		vBeginInfo.renderArea.extent.height = pRenderTarget->getExtent().height;

		getEngine()->getDeviceTable().vkCmdBeginRenderPass(m_vCommandBuffer, &vBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::unbindRenderTarget() const
	{
		getEngine()->getDeviceTable().vkCmdEndRenderPass(m_vCommandBuffer);
	}

	void CommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* pPipeline) const
	{
		getEngine()->getDeviceTable().vkCmdBindPipeline(m_vCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->getPipeline());
	}

	void CommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* pPipeline, const Package* pPackage) const
	{
		// First, bind the packages.
		if (pPackage)
		{
			const auto vDescriptorSet = pPackage->getDescriptorSet();
			getEngine()->getDeviceTable().vkCmdBindDescriptorSets(m_vCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->getPipelineLayout(), pPackage->getSetIndex(), 1, &vDescriptorSet, 0, nullptr);
		}

		// Now we can bind the pipeline.
		getEngine()->getDeviceTable().vkCmdBindPipeline(m_vCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->getPipeline());
	}

	void CommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* pPipeline, const std::vector<Package*>& pPackages) const
	{
		int32_t firstSetIndex = -1;

		// First, bind the packages.
		std::vector<VkDescriptorSet> vDescriptorSets;
		for (const auto pPackage : pPackages)
		{
			// We only need to include the non-nullptr packages.
			if (pPackage)
			{
				vDescriptorSets.emplace_back(pPackage->getDescriptorSet());

				if (firstSetIndex == -1)
					firstSetIndex = pPackage->getSetIndex();
			}
		}

		// Bind the descriptor sets if available.
		if (vDescriptorSets.size())
		{
			getEngine()->getDeviceTable().vkCmdBindDescriptorSets(m_vCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
				pPipeline->getPipelineLayout(), firstSetIndex, static_cast<uint32_t>(vDescriptorSets.size()), vDescriptorSets.data(), 0, nullptr);
		}

		// Now we can bind the pipeline.
		getEngine()->getDeviceTable().vkCmdBindPipeline(m_vCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->getPipeline());
	}

	void CommandBuffer::bindVertexBuffer(const Buffer* pVertexBuffer) const
	{
		// Validate the buffer type.
		if (pVertexBuffer->getType() != BufferType::Vertex)
			throw BackendError("Cannot bind the buffer as a Vertex buffer! The types does not match.");

		// Now we can bind it.
		constexpr std::array<VkDeviceSize, 1> offset = { 0 };
		const auto vBuffer = pVertexBuffer->getBuffer();

		getEngine()->getDeviceTable().vkCmdBindVertexBuffers(m_vCommandBuffer, 0, 1, &vBuffer, offset.data());
	}

	void CommandBuffer::bindIndexBuffer(const Buffer* pIndexBuffer, const VkIndexType indexType) const
	{
		// Validate the buffer type.
		if (pIndexBuffer->getType() != BufferType::Index)
			throw BackendError("Cannot bind the buffer as a Index buffer! The types does not match.");

		// Now we can bind it.
		getEngine()->getDeviceTable().vkCmdBindIndexBuffer(m_vCommandBuffer, pIndexBuffer->getBuffer(), 0, indexType);
	}

	void CommandBuffer::bindViewport(const VkViewport viewport) const
	{
		getEngine()->getDeviceTable().vkCmdSetViewport(m_vCommandBuffer, 0, 1, &viewport);
	}

	void CommandBuffer::bindScissor(const VkRect2D scissor) const
	{
		getEngine()->getDeviceTable().vkCmdSetScissor(m_vCommandBuffer, 0, 1, &scissor);
	}

	void CommandBuffer::drawVertices(const uint32_t vertexCount) const
	{
		getEngine()->getDeviceTable().vkCmdDraw(m_vCommandBuffer, vertexCount, 1, 0, 0);
	}

	void CommandBuffer::drawIndices(const uint32_t indexCount, const uint32_t vertexOffset) const
	{
		getEngine()->getDeviceTable().vkCmdDrawIndexed(m_vCommandBuffer, indexCount, 1, 0, vertexOffset, 0);
	}

	void CommandBuffer::end()
	{
		// Just return if we are not recording.
		if (!isRecording())
			return;

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkEndCommandBuffer(m_vCommandBuffer), "Failed to end command buffer recording!");
		m_bIsRecording = false;
	}

	void CommandBuffer::submit(bool shouldWait)
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

			FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateFence(getEngine()->getLogicalDevice(), &vFenceCreateInfo, nullptr, &vFence), "Failed to create the synchronization fence!");
		}

		// Submit the queue.
		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkQueueSubmit(queue.getQueue(), 1, &vSubmitInfo, vFence), "Failed to submit the queue!");

		// Destroy the fence if we created it.
		if (shouldWait)
		{
			FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkWaitForFences(getEngine()->getLogicalDevice(), 1, &vFence, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait for the fence!");
			getEngine()->getDeviceTable().vkDestroyFence(getEngine()->getLogicalDevice(), vFence, nullptr);
		}
	}

	void CommandBuffer::terminate()
	{
		getEngine()->getDeviceTable().vkFreeCommandBuffers(getEngine()->getLogicalDevice(), m_vCommandPool, 1, &m_vCommandBuffer);
		getEngine()->getDeviceTable().vkDestroySemaphore(getEngine()->getLogicalDevice(), m_vInFlightSemaphore, nullptr);
		getEngine()->getDeviceTable().vkDestroySemaphore(getEngine()->getLogicalDevice(), m_vRenderFinishedSemaphore, nullptr);
		toggleTerminated();
	}
	
	void CommandBuffer::initialize()
	{
		// Create the semaphores.
		createSemaphores();
	}

	void CommandBuffer::createSemaphores()
	{
		// Create the semaphore create info structure.
		VkSemaphoreCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vCreateInfo.pNext = nullptr;
		vCreateInfo.flags = 0;

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateSemaphore(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vInFlightSemaphore), "Failed to create the in flight semaphore!");
		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateSemaphore(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vRenderFinishedSemaphore), "Failed to create the render finished semaphore!");
	}
}