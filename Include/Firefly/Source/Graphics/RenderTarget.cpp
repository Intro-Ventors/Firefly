#include "Firefly/Graphics/RenderTarget.hpp"

namespace Firefly
{
	RenderTarget::RenderTarget(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const VkFormat vColorFormat, const uint8_t frameCount)
		: EngineBoundObject(pEngine), m_Extent(extent), m_FrameCount(frameCount)
	{
		m_vFrameBuffers.resize(frameCount);
		m_pCommandBuffers.reserve(frameCount);

		// Create the attachments.
		m_pColorAttachment = Image::create(pEngine, extent, vColorFormat, ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
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
	
	RenderTarget::~RenderTarget()
	{
		if (!isTerminated())
			terminate();
	}
	
	CommandBuffer* RenderTarget::setupFrame(const std::array<VkClearValue, 2>& vClearColors)
	{
		const auto& pCommandBuffer = m_pCommandBuffers[getFrameIndex()];
		pCommandBuffer->begin();
		bind(pCommandBuffer.get(), vClearColors);

		return pCommandBuffer.get();
	}
	
	void RenderTarget::bind(const CommandBuffer* pCommandBuffer, const std::array<VkClearValue, 2>& vClearColors) const
	{
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
	
	void RenderTarget::unbind(const CommandBuffer* pCommandBuffer) const
	{
		getEngine()->getDeviceTable().vkCmdEndRenderPass(pCommandBuffer->getCommandBuffer());
	}
	
	void RenderTarget::submitFrame(const bool shouldWait)
	{
		auto pCommandBuffer = m_pCommandBuffers[getFrameIndex()];
		unbind(pCommandBuffer.get());

		pCommandBuffer->submit(shouldWait);
		incrementFrameIndex();
	}
	
	std::shared_ptr<RenderTarget> RenderTarget::create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const VkFormat vColorFormat, const uint8_t frameCount)
	{
		return std::make_shared<RenderTarget>(pEngine, extent, vColorFormat, frameCount);
	}
	
	void RenderTarget::terminate()
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
	
	void RenderTarget::createRenderPass()
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
		vAttachmentDescriptions[1].loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		vAttachmentDescriptions[1].storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vAttachmentDescriptions[1].stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vAttachmentDescriptions[1].stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
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
	
	void RenderTarget::createFrameBuffers()
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
	
	void RenderTarget::createCommandPool()
	{
		const auto queue = getEngine()->getQueue(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);

		VkCommandPoolCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vCreateInfo.pNext = VK_NULL_HANDLE;
		vCreateInfo.queueFamilyIndex = queue.getFamily().value();

		Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateCommandPool(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vCommandPool), "Failed to create the command pool!");
	}
	
	void RenderTarget::allocateCommandBuffers()
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
}