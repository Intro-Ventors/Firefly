#pragma once

#include "GraphicsEngine.hpp"
#include "Firefly/Image.hpp"

namespace Firefly
{
	/**
	 * Render target object.
	 * Render targets contain the rendering pipelines and the processing pipelines.
	 */
	class RenderTarget : public EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 * @param extent The frame buffer extent.
		 * @param frameCount The number of frame buffers to use. Default is 2.
		 */
		explicit RenderTarget(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const uint8_t frameCount = 2)
			: EngineBoundObject(pEngine), m_Extent(extent), m_FrameCount(frameCount)
		{
			m_vFrameBuffers.resize(frameCount);
			m_pCommandBuffers.reserve(frameCount);

			// Create the attachments.
			m_pColorAttachment = Image::create(pEngine, extent, VkFormat::VK_FORMAT_R8G8B8A8_SRGB, ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
			m_pDepthAttachment = Image::create(pEngine, extent, getEngine()->findBestDepthFormat(), ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			// Create the render pass.
			createRenderPass();

			// Create the frame buffers.
			createFrameBuffers();

			// Create the command pool.
			createCommandPool();

			// Allocate command buffers.
			allocateCommandBuffers();
		}

		/**
		 * Destructor.
		 */
		~RenderTarget() override
		{
			if (!isTerminated())
				terminate();
		}

		/**
		 * Setup the new frame.
		 * This will submit a new frame to the GPU.
		 *
		 * @return The command buffer pointer.
		 */
		CommandBuffer* setupFrame()
		{
			const auto& pCommandBuffer = m_pCommandBuffers[getFrameIndex()];
			pCommandBuffer->begin();
			bind(pCommandBuffer.get());

			return pCommandBuffer.get();
		}

		/**
		 * Bind the render target to the command buffer.
		 *
		 * @param pCommandBuffer The command buffer pointer to bind to.
		 */
		void bind(const CommandBuffer* pCommandBuffer) const
		{
			// Setup the clear values.
			std::array<VkClearValue, 2> vClearColors;
			vClearColors[0].color.float32[0] = 0.0f;
			vClearColors[0].color.float32[1] = 0.0f;
			vClearColors[0].color.float32[2] = 0.0f;
			vClearColors[0].color.float32[3] = 1.0f;

			vClearColors[1].depthStencil.depth = 1.0f;
			vClearColors[1].depthStencil.stencil = 0;

			// Create the begin info structure.
			VkRenderPassBeginInfo vBeginInfo = {};
			vBeginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			vBeginInfo.pNext = VK_NULL_HANDLE;
			vBeginInfo.renderPass = m_vRenderPass;
			vBeginInfo.framebuffer = m_vFrameBuffers[getFrameIndex()];
			vBeginInfo.clearValueCount = 2;
			vBeginInfo.pClearValues = vClearColors.data();
			vBeginInfo.renderArea.extent.width = m_Extent.width;
			vBeginInfo.renderArea.extent.height = m_Extent.height;

			getEngine()->getDeviceTable().vkCmdBeginRenderPass(pCommandBuffer->getCommandBuffer(), &vBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
		}

		/**
		 * Unbind the render target from the command buffer.
		 *
		 * @param pCommandBuffer The command buffer pointer to unbind from.
		 */
		void unbind(const CommandBuffer* pCommandBuffer) const
		{
			getEngine()->getDeviceTable().vkCmdEndRenderPass(pCommandBuffer->getCommandBuffer());
		}

		/**
		 * Submit the frame to the GPU.
		 *
		 * @param shouldWait Whether or not if we should wait till execution ends. Default is true.
		 */
		void submitFrame(const bool shouldWait = true)
		{
			auto pCommandBuffer = m_pCommandBuffers[getFrameIndex()];
			unbind(pCommandBuffer.get());

			pCommandBuffer->submit(shouldWait);
			incrementFrameIndex();
		}

		/**
		 * Create a new render target object.
		 *
		 * @param pEngine The engine pointer.
		 * @param extent The frame buffer extent.
		 * @param frameCount The number of frame buffers to use. Default is 2.
		 * @return The render target pointer.
		 */
		static std::shared_ptr<RenderTarget> create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const uint8_t frameCount = 2)
		{
			return std::make_shared<RenderTarget>(pEngine, extent, frameCount);
		}

		/**
		 * Terminate the render target.
		 */
		void terminate() override
		{
			for (const auto& pCommandBuffer : m_pCommandBuffers)
				pCommandBuffer->terminate();

			getEngine()->getDeviceTable().vkDestroyCommandPool(getEngine()->getLogicalDevice(), m_vCommandPool, nullptr);
			getEngine()->getDeviceTable().vkDestroyRenderPass(getEngine()->getLogicalDevice(), m_vRenderPass, nullptr);

			for (auto vFrameBuffer : m_vFrameBuffers)
				getEngine()->getDeviceTable().vkDestroyFramebuffer(getEngine()->getLogicalDevice(), vFrameBuffer, nullptr);

			m_vFrameBuffers.clear();
			m_pCommandBuffers.clear();

			m_pColorAttachment->terminate();
			m_pDepthAttachment->terminate();

			toggleTerminated();
		}

		/**
		 * Get the render target extent.
		 *
		 * @return The extent.
		 */
		VkExtent3D getExtent() const { return m_Extent; }

		/**
		 * Get the color attachment.
		 *
		 * @return The color attachment pointer.
		 */
		std::shared_ptr<Image> getColorAttachment() const { return m_pColorAttachment; }

		/**
		 * Get the depth attachment.
		 *
		 * @return The depth attachment pointer.
		 */
		std::shared_ptr<Image> getDepthAttachment() const { return m_pDepthAttachment; }

		/**
		 * Get the frame buffers.
		 *
		 * @return The frame buffers.
		 */
		std::vector<VkFramebuffer> getFrameBuffers() const { return m_vFrameBuffers; }

		/**
		 * Get the render pass.
		 *
		 * @return The render pass.
		 */
		VkRenderPass getRenderPass() const { return m_vRenderPass; }

		/**
		 * Get the frame count.
		 *
		 * @return The frame count.
		 */
		uint8_t getFrameCount() const { return m_FrameCount; }

		/**
		 * Get the frame index.
		 *
		 * @return The frame index.
		 */
		uint8_t getFrameIndex() const { return m_FrameIndex; }

	private:
		/**
		 * Create the render pass.
		 */
		void createRenderPass()
		{
			// Crate attachment descriptions.
			std::array<VkAttachmentDescription, 2> vAttachmentDescriptions;
			vAttachmentDescriptions[0].flags = 0;
			vAttachmentDescriptions[0].format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			vAttachmentDescriptions[0].initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			vAttachmentDescriptions[0].finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			vAttachmentDescriptions[0].loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			vAttachmentDescriptions[0].storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
			vAttachmentDescriptions[0].stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vAttachmentDescriptions[0].stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			vAttachmentDescriptions[0].samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

			vAttachmentDescriptions[1].flags = 0;
			vAttachmentDescriptions[1].format = m_pDepthAttachment->getFormat();
			vAttachmentDescriptions[1].initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			vAttachmentDescriptions[1].finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			vAttachmentDescriptions[1].loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vAttachmentDescriptions[1].storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			vAttachmentDescriptions[1].stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			vAttachmentDescriptions[1].stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
			vAttachmentDescriptions[1].samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

			// Create the subpass dependencies.
			std::array<VkSubpassDependency, 2> vSubpassDependencies;
			vSubpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			vSubpassDependencies[0].dstSubpass = 0;
			vSubpassDependencies[0].srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			vSubpassDependencies[0].dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			vSubpassDependencies[0].srcAccessMask = VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
			vSubpassDependencies[0].dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			vSubpassDependencies[0].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

			vSubpassDependencies[1].srcSubpass = 0;
			vSubpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			vSubpassDependencies[1].srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			vSubpassDependencies[1].dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			vSubpassDependencies[1].srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			vSubpassDependencies[1].dstAccessMask = VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
			vSubpassDependencies[1].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

			// Create the subpass description.
			VkAttachmentReference vColorAttachmentReference = {};
			vColorAttachmentReference.attachment = 0;
			vColorAttachmentReference.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference vDepthAttachmentReference = {};
			vDepthAttachmentReference.attachment = 1;
			vDepthAttachmentReference.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription vSubpassDescription = {};
			vSubpassDescription.flags = 0;
			vSubpassDescription.colorAttachmentCount = 1;
			vSubpassDescription.pColorAttachments = &vColorAttachmentReference;
			vSubpassDescription.pResolveAttachments = nullptr;
			vSubpassDescription.pDepthStencilAttachment = &vDepthAttachmentReference;
			vSubpassDescription.inputAttachmentCount = 0;
			vSubpassDescription.pInputAttachments = nullptr;
			vSubpassDescription.preserveAttachmentCount = 0;
			vSubpassDescription.pPreserveAttachments = nullptr;
			vSubpassDescription.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;

			// Create the render target.
			VkRenderPassCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.flags = 0;
			vCreateInfo.attachmentCount = 2;
			vCreateInfo.pAttachments = vAttachmentDescriptions.data();
			vCreateInfo.dependencyCount = 2;
			vCreateInfo.pDependencies = vSubpassDependencies.data();
			vCreateInfo.subpassCount = 1;
			vCreateInfo.pSubpasses = &vSubpassDescription;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateRenderPass(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vRenderPass), "Failed to create render pass!");
		}

		/**
		 * Create the frame buffers.
		 */
		void createFrameBuffers()
		{
			VkFramebufferCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			vCreateInfo.flags = 0;
			vCreateInfo.pNext = VK_NULL_HANDLE;
			vCreateInfo.layers = 1;
			vCreateInfo.renderPass = m_vRenderPass;
			vCreateInfo.width = m_Extent.width;
			vCreateInfo.height = m_Extent.height;
			vCreateInfo.attachmentCount = 2;

			// Iterate and create the frame buffers.
			for (uint8_t i = 0; i < m_FrameCount; i++)
			{
				std::array<VkImageView, 2> vImageViews;
				vImageViews[0] = m_pColorAttachment->getImageView();
				vImageViews[1] = m_pDepthAttachment->getImageView();

				vCreateInfo.pAttachments = vImageViews.data();
				Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateFramebuffer(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vFrameBuffers[i]), "Failed to create the frame buffer!");
			}
		}

		/**
		 * Create the command pool.
		 */
		void createCommandPool()
		{
			const auto queue = getEngine()->getQueue(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);

			VkCommandPoolCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			vCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			vCreateInfo.pNext = VK_NULL_HANDLE;
			vCreateInfo.queueFamilyIndex = queue.getFamily().value();

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateCommandPool(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vCommandPool), "Failed to create the command pool!");
		}

		/**
		 * Allocate command buffers.
		 */
		void allocateCommandBuffers()
		{
			// Create the allocate info structure.
			VkCommandBufferAllocateInfo vAllocateInfo = {};
			vAllocateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			vAllocateInfo.pNext = VK_NULL_HANDLE;
			vAllocateInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			vAllocateInfo.commandPool = m_vCommandPool;
			vAllocateInfo.commandBufferCount = m_FrameCount;

			std::vector<VkCommandBuffer> vCommandBuffers(m_FrameCount);
			Utility::ValidateResult(getEngine()->getDeviceTable().vkAllocateCommandBuffers(getEngine()->getLogicalDevice(), &vAllocateInfo, vCommandBuffers.data()), "Failed to allocate command buffer!");

			// Create the command buffers.
			for (const auto vCommandBuffer : vCommandBuffers)
				m_pCommandBuffers.emplace_back(CommandBuffer::create(getEngine(), m_vCommandPool, vCommandBuffer));
		}

		/**
		 * Increment the frame index.
		 */
		void incrementFrameIndex() { m_FrameIndex = ++m_FrameIndex % m_FrameCount; }

	private:
		const VkExtent3D m_Extent;

		std::shared_ptr<Image> m_pColorAttachment = nullptr;
		std::shared_ptr<Image> m_pDepthAttachment = nullptr;
		std::vector<VkFramebuffer> m_vFrameBuffers;
		std::vector<std::shared_ptr<CommandBuffer>> m_pCommandBuffers;

		VkRenderPass m_vRenderPass = VK_NULL_HANDLE;
		VkCommandPool m_vCommandPool = VK_NULL_HANDLE;

		const uint8_t m_FrameCount = 0;
		uint8_t m_FrameIndex = 0;
	};
}