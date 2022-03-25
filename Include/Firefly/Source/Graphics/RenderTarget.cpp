#include "Firefly/Graphics/RenderTarget.hpp"

#include <array>

namespace Firefly
{
	std::vector<VkClearValue> CreateClearValues(const float r /*= 0.0f*/, const float g /*= 0.0f*/, const float b /*= 0.0f*/, const float a /*= 1.0f*/, const float depth /*= 1.0f*/, const uint32_t stencil /*= 0*/)
	{
		std::vector<VkClearValue> vClearColors(2);
		vClearColors[0].color.float32[0] = r;
		vClearColors[0].color.float32[1] = g;
		vClearColors[0].color.float32[2] = b;
		vClearColors[0].color.float32[3] = a;

		vClearColors[1].depthStencil.depth = depth;
		vClearColors[1].depthStencil.stencil = stencil;

		return vClearColors;
	}

	RenderTarget::RenderTarget(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const uint8_t frameCount)
		: EngineBoundObject(pEngine), m_Extent(extent), m_FrameCount(frameCount)
	{

	}

	RenderTarget::~RenderTarget()
	{
		if (!isTerminated())
			terminate();
	}

	std::shared_ptr<RenderTarget> RenderTarget::create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const VkFormat vColorFormat, const uint8_t frameCount)
	{
		const auto pointer = std::make_shared<RenderTarget>(pEngine, extent, frameCount);
		pointer->initialize(vColorFormat);

		return pointer;
	}

	CommandBuffer* RenderTarget::setupFrame(const std::vector<VkClearValue>& vClearColors)
	{
		const auto& pCommandBuffer = m_pCommandBuffers[getFrameIndex()];
		pCommandBuffer->begin();
		pCommandBuffer->bindRenderTarget(this, vClearColors);

		return pCommandBuffer.get();
	}

	void RenderTarget::submitFrame(const bool shouldWait)
	{
		auto pCommandBuffer = m_pCommandBuffers[getFrameIndex()];
		pCommandBuffer->unbindRenderTarget();

		pCommandBuffer->submit(shouldWait);
		incrementFrameIndex();
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
		VkRenderPassCreateInfo vRenderPassCreateInfo = {};
		vRenderPassCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		vRenderPassCreateInfo.pNext = nullptr;
		vRenderPassCreateInfo.flags = 0;
		vRenderPassCreateInfo.attachmentCount = 2;
		vRenderPassCreateInfo.pAttachments = vAttachmentDescriptions.data();
		vRenderPassCreateInfo.dependencyCount = 2;
		vRenderPassCreateInfo.pDependencies = vSubpassDependencies.data();
		vRenderPassCreateInfo.subpassCount = 1;
		vRenderPassCreateInfo.pSubpasses = &vSubpassDescription;

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateRenderPass(getEngine()->getLogicalDevice(), &vRenderPassCreateInfo, nullptr, &m_vRenderPass), "Failed to create render pass!");
	}
	
	void RenderTarget::createFramebuffer()
	{
		VkFramebufferCreateInfo vFramebufferCreateInfo = {};
		vFramebufferCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		vFramebufferCreateInfo.flags = 0;
		vFramebufferCreateInfo.pNext = VK_NULL_HANDLE;
		vFramebufferCreateInfo.layers = 1;
		vFramebufferCreateInfo.renderPass = m_vRenderPass;
		vFramebufferCreateInfo.width = m_Extent.width;
		vFramebufferCreateInfo.height = m_Extent.height;
		vFramebufferCreateInfo.attachmentCount = 2;

		// Iterate and create the frame buffers.
		for (uint8_t i = 0; i < m_FrameCount; i++)
		{
			std::array<VkImageView, 2> vImageViews;
			vImageViews[0] = m_pColorAttachment->getImageView();
			vImageViews[1] = m_pDepthAttachment->getImageView();

			vFramebufferCreateInfo.pAttachments = vImageViews.data();
			FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateFramebuffer(getEngine()->getLogicalDevice(), &vFramebufferCreateInfo, nullptr, &m_vFrameBuffers[i]), "Failed to create the frame buffer!");
		}
	}

	void RenderTarget::createCommandPool()
	{
		const auto queue = getEngine()->getQueue(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);

		VkCommandPoolCreateInfo vCommandPoolCreateInfo = {};
		vCommandPoolCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vCommandPoolCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vCommandPoolCreateInfo.pNext = VK_NULL_HANDLE;
		vCommandPoolCreateInfo.queueFamilyIndex = queue.getFamily().value();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateCommandPool(getEngine()->getLogicalDevice(), &vCommandPoolCreateInfo, nullptr, &m_vCommandPool), "Failed to create the command pool!");
	}

	void RenderTarget::allocateCommandBuffer()
	{
		// Create the allocate info structure.
		VkCommandBufferAllocateInfo vAllocateInfo = {};
		vAllocateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vAllocateInfo.pNext = VK_NULL_HANDLE;
		vAllocateInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vAllocateInfo.commandPool = m_vCommandPool;
		vAllocateInfo.commandBufferCount = m_FrameCount;

		std::vector<VkCommandBuffer> vCommandBuffers(m_FrameCount);
		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkAllocateCommandBuffers(getEngine()->getLogicalDevice(), &vAllocateInfo, vCommandBuffers.data()), "Failed to allocate command buffer!");

		// Create the command buffers.
		for (const auto vCommandBuffer : vCommandBuffers)
			m_pCommandBuffers.emplace_back(CommandBuffer::create(getEngine(), m_vCommandPool, vCommandBuffer));
	}
	
	void RenderTarget::initialize(const VkFormat vColorFormat)
	{
		m_vFrameBuffers.resize(m_FrameCount);
		m_pCommandBuffers.reserve(m_FrameCount);

		// Create the attachments.
		m_pColorAttachment = Image::create(getEngine(), m_Extent, vColorFormat, ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		m_pColorAttachment->changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		m_pDepthAttachment = Image::create(getEngine(), m_Extent, getEngine()->findBestDepthFormat(), ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

		// Create the render pass.
		createRenderPass();

		// Create the frame buffers.
		createFramebuffer();

		// Create the command pool.
		createCommandPool();

		// Allocate command buffers.
		allocateCommandBuffer();
	}
}