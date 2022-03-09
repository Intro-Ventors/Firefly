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

			// Create the attachments.
			m_pColorAttachment = Image::create(pEngine, extent, VkFormat::VK_FORMAT_R8G8B8A8_SRGB, ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
			m_pDepthAttachment = Image::create(pEngine, extent, getEngine()->findBestDepthFormat(), ImageType::TwoDimension, 1, VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			// Create the render pass.
			createRenderPass();

			// Create the frame buffers.
			createFrameBuffers();
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
			getEngine()->getDeviceTable().vkDestroyRenderPass(getEngine()->getLogicalDevice(), m_vRenderPass, nullptr);

			for (auto vFrameBuffer : m_vFrameBuffers)
				getEngine()->getDeviceTable().vkDestroyFramebuffer(getEngine()->getLogicalDevice(), vFrameBuffer, nullptr);

			m_vFrameBuffers.clear();

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

	private:
		const VkExtent3D m_Extent;

		std::shared_ptr<Image> m_pColorAttachment = nullptr;
		std::shared_ptr<Image> m_pDepthAttachment = nullptr;
		std::vector<VkFramebuffer> m_vFrameBuffers;

		VkRenderPass m_vRenderPass = VK_NULL_HANDLE;

		const uint8_t m_FrameCount = 0;
		uint8_t m_FrameIndex = 0;
	};
}