#pragma once

#include "GraphicsEngine.hpp"
#include "Firefly/Image.hpp"

#include <array>

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
	constexpr std::array<VkClearValue, 2> CreateClearValues(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f, const float a = 1.0f, const float depth = 1.0f, const uint32_t stencil = 0)
	{
		std::array<VkClearValue, 2> vClearColors{};
		vClearColors[0].color.float32[0] = r;
		vClearColors[0].color.float32[1] = g;
		vClearColors[0].color.float32[2] = b;
		vClearColors[0].color.float32[3] = a;

		vClearColors[1].depthStencil.depth = depth;
		vClearColors[1].depthStencil.stencil = stencil;

		return vClearColors;
	}

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
		 * @param vColorFormat The color format to use.
		 * @param frameCount The number of frame buffers to use. Default is 2.
		 */
		explicit RenderTarget(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const VkFormat vColorFormat, const uint8_t frameCount = 2);

		/**
		 * Destructor.
		 */
		~RenderTarget() override;

		/**
		 * Setup the new frame.
		 * This will submit a new frame to the GPU.
		 *
		 * @return The command buffer pointer.
		 */
		CommandBuffer* setupFrame(const std::array<VkClearValue, 2>& vClearColors);

		/**
		 * Bind the render target to the command buffer.
		 *
		 * @param pCommandBuffer The command buffer pointer to bind to.
		 */
		void bind(const CommandBuffer* pCommandBuffer, const std::array<VkClearValue, 2>& vClearColors) const;

		/**
		 * Unbind the render target from the command buffer.
		 *
		 * @param pCommandBuffer The command buffer pointer to unbind from.
		 */
		void unbind(const CommandBuffer* pCommandBuffer) const;

		/**
		 * Submit the frame to the GPU.
		 *
		 * @param shouldWait Whether or not if we should wait till execution ends. Default is true.
		 */
		void submitFrame(const bool shouldWait = true);

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
		void createRenderPass();

		/**
		 * Create the frame buffers.
		 */
		void createFrameBuffers();

		/**
		 * Create the command pool.
		 */
		void createCommandPool();

		/**
		 * Allocate command buffers.
		 */
		void allocateCommandBuffers();

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