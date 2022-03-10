#pragma once

#include "GraphicsEngine.hpp"
#include "RenderTarget.hpp"
#include "Firefly/Shader.hpp"

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
		 * @param pipelineName The unique name given to the pipeline. This will be used for caching.
		 * @param pShaders The shaders used by the pipeline.
		 * @param pRenderTarget The render target pointer to which this pipeline is bound to.
		 */
		explicit GraphicsPipeline(const std::shared_ptr<GraphicsEngine>& pEngine, const std::string& pipelineName, const std::vector<std::shared_ptr<Shader>>& pShaders, const std::shared_ptr<RenderTarget>& pRenderTarget)
			: EngineBoundObject(pEngine), m_Name(pipelineName), m_pShaders(pShaders), m_pRenderTarget(pRenderTarget)
		{
			// Create the pipeline layout.
			createPipelineLayout();
		}

		/**
		 * Destructor.
		 */
		~GraphicsPipeline() override
		{
			if (!isTerminated())
				terminate();
		}

		/**
		 * Terminate the pipeline.
		 */
		void terminate() override
		{
			getEngine()->getDeviceTable().vkDestroyPipelineLayout(getEngine()->getLogicalDevice(), m_vPipelineLayout, nullptr);
			toggleTerminated();
		}

	private:
		/**
		 * Create the pipeline layout.
		 */
		void createPipelineLayout()
		{
			// Get the descriptor set layouts.
			std::vector<VkDescriptorSetLayout> vDescriptorSetLayouts;
			vDescriptorSetLayouts.reserve(m_pShaders.size());

			for (const auto& pShader : m_pShaders)
				vDescriptorSetLayouts.emplace_back(pShader->getDescriptorSetLayout());

			// Create the pipeline layout.
			VkPipelineLayoutCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			vCreateInfo.flags = 0;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.setLayoutCount = static_cast<uint32_t>(m_pShaders.size());
			vCreateInfo.pSetLayouts = vDescriptorSetLayouts.data();
			vCreateInfo.pushConstantRangeCount = 0;
			vCreateInfo.pPushConstantRanges = nullptr;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreatePipelineLayout(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vPipelineLayout), "Failed to create the pipeline layout!");
		}

	private:
		std::string m_Name;
		std::vector<std::shared_ptr<Shader>> m_pShaders;

		std::shared_ptr<RenderTarget> m_pRenderTarget = nullptr;

		VkPipelineLayout m_vPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_vPipeline = VK_NULL_HANDLE;
		VkPipelineCache m_vPipelineCache = VK_NULL_HANDLE;
	};
}