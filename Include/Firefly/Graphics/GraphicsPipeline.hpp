#pragma once

#include "GraphicsEngine.hpp"
#include "Firefly/EngineBoundObject.hpp"

namespace Firefly
{
	/**
	 * Graphics pipeline object.
	 * The graphics pipeline is used to render data to a render target and specifies all the rendering steps.
	 */
	class GraphicsPipeline final : public EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 */
		explicit GraphicsPipeline(const std::shared_ptr<GraphicsEngine>& pEngine)
			: EngineBoundObject(pEngine)
		{
		}
	};
}