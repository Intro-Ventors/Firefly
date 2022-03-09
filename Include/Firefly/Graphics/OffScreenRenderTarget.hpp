#pragma once

#include "RenderTarget.hpp"

namespace Firefly
{
	/**
	 * Off screen render target object.
	 * Off screen render targets renders images to off screen images, and they are in return can be used for processing.
	 */
	class OffScreenRenderTarget final : public RenderTarget
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 * @param extent The frame buffer extent.
		 * @param frameCount The number of frame buffers to use. Default is 2.
		 */
		explicit OffScreenRenderTarget(const std::shared_ptr<GraphicsEngine>& pEngine, const VkExtent3D extent, const uint8_t frameCount = 2)
			: RenderTarget(pEngine, extent, frameCount)
		{

		}
	};
}