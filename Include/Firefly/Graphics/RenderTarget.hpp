#pragma once

#include "GraphicsEngine.hpp"
#include "Firefly/Image.hpp"

namespace Firefly
{
	/**
	 * Create a color from 0 - 256.
	 *
	 * @param color The color.
	 * @return The compatible color value.
	 */
	constexpr float CreateColor256(const float color) { return color / 256.0f; }

	/**
	 * Create clear color values from the primitives.
	 * Make sure that the color value is compatible. We advice you to use the CreateColor256() function to get the correct value.
	 *
	 * @param r The red value. Default is 0.0f.
	 * @param g The green value. Default is 0.0f.
	 * @param b The blue value. Default is 0.0f.
	 * @param a The alpha value. Default is 1.0f.
	 * @param depth The depth value. Default is 1.0f.
	 * @param stencil The stencil value. Default is 0.
	 */
	std::vector<VkClearValue> CreateClearValues(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f, const float a = 1.0f, const float depth = 1.0f, const uint32_t stencil = 0);

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
		 * 
		 * @param frameCount The number of frame buffers to use. Default is 2.
		 */
		explicit RenderTarget(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const uint8_t frameCount = 2);

		/**
		 * Destructor.
		 */
		~RenderTarget() override;

		/**
		 * Create a new render target object.
		 *
		 * @param pEngine The engine pointer.
		 * @param extent The frame buffer extent.
		 * @param vColorFormat The color format to use.
		 * @param frameCount The number of frame buffers to use. Default is 2.
		 * @return The render target pointer.
		 */
		static std::shared_ptr<RenderTarget> create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const VkFormat vColorFormat, const uint8_t frameCount = 2);

		/**
		 * Setup the new frame.
		 * This will submit a new frame to the GPU.
		 *
		 * @return The command buffer pointer.
		 */
		CommandBuffer* setupFrame(const std::vector<VkClearValue>& vClearColors);

		/**
		 * Submit the frame to the GPU.
		 *
		 * @param shouldWait Whether or not if we should wait till execution ends. Default is true.
		 */
		void submitFrame(const bool shouldWait = true);

		/**
		 * Terminate the render target.
		 */
		void terminate() override;

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
		 * Get the current frame buffer.
		 *
		 * @return The frame buffer in the current frame index.
		 */
		VkFramebuffer getCurrentFrameBuffer() const { return m_vFrameBuffers[getFrameIndex()]; }

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

		/**
		 * Increment the frame index.
		 */
		void incrementFrameIndex() { m_FrameIndex = ++m_FrameIndex % m_FrameCount; }

	private:
		/**
		 * Create the render pass.
		 */
		void createRenderPass();

		/**
		 * Create the frame buffer.
		 */
		void createFramebuffer();

		/**
		 * Create the command pool.
		 */
		void createCommandPool();

		/**
		 * Allocate the command buffer.
		 */
		void allocateCommandBuffer();

		/**
		 * Initialize the render target.
		 * 
		 * @param vColorFormat The color format to use.
		 */
		void initialize(const VkFormat vColorFormat);

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