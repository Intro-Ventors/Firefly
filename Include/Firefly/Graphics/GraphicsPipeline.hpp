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

			// Create the pipeline.
			createPipeline();
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
		 * Create a new graphics pipeline.
		 *
		 * @param pEngine The engine pointer.
		 * @param pipelineName The unique name given to the pipeline. This will be used for caching.
		 * @param pShaders The shaders used by the pipeline.
		 * @param pRenderTarget The render target pointer to which this pipeline is bound to.
		 * @return The graphics pipeline.
		 */
		static std::shared_ptr<GraphicsPipeline> create(const std::shared_ptr<GraphicsEngine>& pEngine, const std::string& pipelineName, const std::vector<std::shared_ptr<Shader>>& pShaders, const std::shared_ptr<RenderTarget>& pRenderTarget)
		{
			return std::make_shared<GraphicsPipeline>(pEngine, pipelineName, pShaders, pRenderTarget);
		}

		/**
		 * Terminate the pipeline.
		 */
		void terminate() override
		{
			getEngine()->getDeviceTable().vkDestroyPipelineLayout(getEngine()->getLogicalDevice(), m_vPipelineLayout, nullptr);
			getEngine()->getDeviceTable().vkDestroyPipeline(getEngine()->getLogicalDevice(), m_vPipeline, nullptr);
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
			std::vector<VkPushConstantRange> vPushConstants;
			vDescriptorSetLayouts.reserve(m_pShaders.size());

			for (const auto& pShader : m_pShaders)
			{
				vDescriptorSetLayouts.emplace_back(pShader->getDescriptorSetLayout());

				const auto& pushConstants = pShader->getPushConstants();
				vPushConstants.insert(vPushConstants.end(), pushConstants.begin(), pushConstants.end());
			}

			// Create the pipeline layout.
			VkPipelineLayoutCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			vCreateInfo.flags = 0;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.setLayoutCount = static_cast<uint32_t>(m_pShaders.size());
			vCreateInfo.pSetLayouts = vDescriptorSetLayouts.data();
			vCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(vPushConstants.size());
			vCreateInfo.pPushConstantRanges = vPushConstants.data();

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreatePipelineLayout(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vPipelineLayout), "Failed to create the pipeline layout!");
		}

		/**
		 * Get the shader stage flag bits from a shader.
		 *
		 * @param pShader the shader pointer.
		 * @return The shader stage flag bits.
		 */
		VkShaderStageFlagBits getStageFlagBits(const Shader* pShader) const
		{
			const auto flags = pShader->getFlags();

			if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
				return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;

			if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
				return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

			if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
				return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

			if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT)
				return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;

			if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
				return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;

			throw BackendError("Unsupported shader type!");
		}

		/**
		 * Get the Vulkan format from the input size.
		 *
		 * @param size The size of the data type.
		 * @return The Vulkan format.
		 */
		VkFormat getFormatFromSize(const uint32_t size) const
		{
			switch (size)
			{
			case 4:
				return VkFormat::VK_FORMAT_R32_UINT;

			case 8:
				return VkFormat::VK_FORMAT_R32G32_SFLOAT;

			case 12:
				return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;

			case 16:
				return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;

			default:
				throw BackendError("Invalid or unsupported shader attribute type!");
			}

			return VkFormat::VK_FORMAT_UNDEFINED;
		}

		/**
		 * Create the pipeline.
		 */
		void createPipeline()
		{
			// Resolve shader info.
			std::vector<VkPipelineShaderStageCreateInfo> vShaderStageCreateInfos;
			vShaderStageCreateInfos.reserve(m_pShaders.size());

			std::vector<VkVertexInputAttributeDescription> vAttributeDescriptions;
			VkVertexInputBindingDescription vBindingDescription = {};

			VkPipelineShaderStageCreateInfo vShaderStageCreateInfo = {};
			vShaderStageCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vShaderStageCreateInfo.flags = 0;
			vShaderStageCreateInfo.pNext = nullptr;
			vShaderStageCreateInfo.pSpecializationInfo = nullptr;
			vShaderStageCreateInfo.pName = "main";

			// Iterate over the shaders and resolve information.
			for (const auto& pShader : m_pShaders)
			{
				vShaderStageCreateInfo.module = pShader->getShaderModule();
				vShaderStageCreateInfo.stage = getStageFlagBits(pShader.get());

				vShaderStageCreateInfos.emplace_back(vShaderStageCreateInfo);

				// Resolve inputs if its the vertex shader.
				if (vShaderStageCreateInfo.stage == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
				{
					const auto& inputs = pShader->getInputAttributes();
					vAttributeDescriptions.reserve(inputs.size());

					VkVertexInputAttributeDescription vAttributeDescription = {};
					vAttributeDescription.binding = 0;
					vAttributeDescription.offset = 0;

					// Resolve the individual attributes.
					for (const auto& attribute : inputs)
					{
						vAttributeDescription.location = attribute.m_Location;
						vAttributeDescription.format = getFormatFromSize(attribute.m_Size);

						vAttributeDescriptions.emplace_back(vAttributeDescription);
						vAttributeDescription.offset += attribute.m_Size;
					}

					vBindingDescription.binding = 0;
					vBindingDescription.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
					vBindingDescription.stride = vAttributeDescription.offset;
				}
			}

			// Setup vertex input state.
			VkPipelineVertexInputStateCreateInfo vVertexInputStateCreateInfo = {};
			vVertexInputStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vVertexInputStateCreateInfo.pNext = VK_NULL_HANDLE;
			vVertexInputStateCreateInfo.flags = 0;
			vVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vAttributeDescriptions.size());
			vVertexInputStateCreateInfo.pVertexAttributeDescriptions = vAttributeDescriptions.data();
			vVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
			vVertexInputStateCreateInfo.pVertexBindingDescriptions = &vBindingDescription;

			// Setup input assembly state.
			VkPipelineInputAssemblyStateCreateInfo vInputAssemblyStateCreateInfo = {};
			vInputAssemblyStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			vInputAssemblyStateCreateInfo.pNext = VK_NULL_HANDLE;
			vInputAssemblyStateCreateInfo.flags = 0;
			vInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
			vInputAssemblyStateCreateInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			// Setup tessellation state.
			VkPipelineTessellationStateCreateInfo vTessellationStateCreateInfo = {};
			vTessellationStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			vTessellationStateCreateInfo.pNext = VK_NULL_HANDLE;
			vTessellationStateCreateInfo.flags = 0;
			vTessellationStateCreateInfo.patchControlPoints = 0;

			// Resolve viewport state.
			VkRect2D vRect = {};
			vRect.extent.width = m_pRenderTarget->getExtent().width;
			vRect.extent.height = m_pRenderTarget->getExtent().height;
			vRect.offset = { 0, 0 };

			VkViewport vViewport = {};
			vViewport.width = static_cast<float>(vRect.extent.width);
			vViewport.height = static_cast<float>(vRect.extent.height);
			vViewport.maxDepth = 1.0f;
			vViewport.minDepth = 0.0f;
			vViewport.x = 0.0f;
			vViewport.y = 0.0f;

			VkPipelineViewportStateCreateInfo vViewportStateCreateInfo = {};
			vViewportStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			vViewportStateCreateInfo.pNext = VK_NULL_HANDLE;
			vViewportStateCreateInfo.flags = 0;
			vViewportStateCreateInfo.scissorCount = 1;
			vViewportStateCreateInfo.pScissors = &vRect;
			vViewportStateCreateInfo.viewportCount = 1;
			vViewportStateCreateInfo.pViewports = &vViewport;

			// Setup color blend state.
			VkPipelineColorBlendAttachmentState vColorBlendAttachmentState = {};
			vColorBlendAttachmentState.blendEnable = false;
			vColorBlendAttachmentState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
			vColorBlendAttachmentState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
			vColorBlendAttachmentState.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
			vColorBlendAttachmentState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
			vColorBlendAttachmentState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
			vColorBlendAttachmentState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
			vColorBlendAttachmentState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;

			VkPipelineColorBlendStateCreateInfo vColorBlendStateCreateInfo = {};
			vColorBlendStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			vColorBlendStateCreateInfo.pNext = VK_NULL_HANDLE;
			vColorBlendStateCreateInfo.flags = 0;
			vColorBlendStateCreateInfo.logicOp = VkLogicOp::VK_LOGIC_OP_CLEAR;
			vColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
			vColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
			vColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
			vColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
			vColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
			vColorBlendStateCreateInfo.attachmentCount = 1;
			vColorBlendStateCreateInfo.pAttachments = &vColorBlendAttachmentState;

			// Setup rasterization state.
			VkPipelineRasterizationStateCreateInfo vRasterizationStateCreateInfo = {};
			vRasterizationStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			vRasterizationStateCreateInfo.pNext = VK_NULL_HANDLE;
			vRasterizationStateCreateInfo.flags = 0;
			vRasterizationStateCreateInfo.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
			vRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
			vRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
			vRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
			vRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
			vRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
			vRasterizationStateCreateInfo.frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
			vRasterizationStateCreateInfo.lineWidth = 1.0f;
			vRasterizationStateCreateInfo.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
			vRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;

			// Setup multisample state.
			VkPipelineMultisampleStateCreateInfo vMultisampleStateCreateInfo = {};
			vMultisampleStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			vMultisampleStateCreateInfo.pNext = VK_NULL_HANDLE;
			vMultisampleStateCreateInfo.flags = 0;
			vMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
			vMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
			vMultisampleStateCreateInfo.minSampleShading = 1.0f;
			//vMultisampleStateCreateInfo.pSampleMask;	// TODO
			vMultisampleStateCreateInfo.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
			vMultisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;

			// Setup depth stencil state.
			VkPipelineDepthStencilStateCreateInfo vDepthStencilStateCreateInfo = {};
			vDepthStencilStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			vDepthStencilStateCreateInfo.pNext = VK_NULL_HANDLE;
			vDepthStencilStateCreateInfo.flags = 0;
			vDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
			vDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
			vDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
			vDepthStencilStateCreateInfo.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;

			// Setup dynamic state.
			std::array<VkDynamicState, 2> vDynamicStates;
			vDynamicStates[0] = VkDynamicState::VK_DYNAMIC_STATE_SCISSOR;
			vDynamicStates[1] = VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT;

			VkPipelineDynamicStateCreateInfo vDynamicStateCreateInfo = {};
			vDynamicStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			vDynamicStateCreateInfo.pNext = VK_NULL_HANDLE;
			vDynamicStateCreateInfo.flags = 0;
			vDynamicStateCreateInfo.dynamicStateCount = 2;
			vDynamicStateCreateInfo.pDynamicStates = vDynamicStates.data();

			// Setup pipeline create info.
			VkGraphicsPipelineCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			vCreateInfo.pNext = VK_NULL_HANDLE;
			vCreateInfo.flags = 0;
			vCreateInfo.stageCount = static_cast<uint32_t>(vShaderStageCreateInfos.size());
			vCreateInfo.pStages = vShaderStageCreateInfos.data();
			vCreateInfo.pVertexInputState = &vVertexInputStateCreateInfo;
			vCreateInfo.pInputAssemblyState = &vInputAssemblyStateCreateInfo;
			vCreateInfo.pTessellationState = &vTessellationStateCreateInfo;
			vCreateInfo.pViewportState = &vViewportStateCreateInfo;
			vCreateInfo.pRasterizationState = &vRasterizationStateCreateInfo;
			vCreateInfo.pMultisampleState = &vMultisampleStateCreateInfo;
			vCreateInfo.pDepthStencilState = &vDepthStencilStateCreateInfo;
			vCreateInfo.pColorBlendState = &vColorBlendStateCreateInfo;
			vCreateInfo.pDynamicState = &vDynamicStateCreateInfo;
			vCreateInfo.layout = m_vPipelineLayout;
			vCreateInfo.renderPass = m_pRenderTarget->getRenderPass();
			vCreateInfo.subpass = 0;
			vCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			vCreateInfo.basePipelineIndex = 0;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateGraphicsPipelines(getEngine()->getLogicalDevice(), m_vPipelineCache, 1, &vCreateInfo, nullptr, &m_vPipeline), "Failed to create the graphics pipeline!");
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