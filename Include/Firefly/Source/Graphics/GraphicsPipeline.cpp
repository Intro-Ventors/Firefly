#include "Firefly/Graphics/GraphicsPipeline.hpp"

#include <array>

namespace Firefly
{
	GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<GraphicsEngine>& pEngine, const std::string& pipelineName, const std::vector<std::shared_ptr<Shader>>& pShaders, const std::shared_ptr<RenderTarget>& pRenderTarget, const GraphicsPipelineSpecification& specification)
		: EngineBoundObject(pEngine), m_Name(pipelineName), m_pShaders(pShaders), m_pRenderTarget(pRenderTarget), m_Specification(specification)
	{
		// Create the pipeline layout.
		createPipelineLayout();

		// Create the pipeline.
		createPipeline();
	}
	
	GraphicsPipeline::~GraphicsPipeline()
	{
		if (!isTerminated())
			terminate();
	}
	
	std::shared_ptr<GraphicsPipeline> GraphicsPipeline::create(const std::shared_ptr<GraphicsEngine>& pEngine, const std::string& pipelineName, const std::vector<std::shared_ptr<Shader>>& pShaders, const std::shared_ptr<RenderTarget>& pRenderTarget, const GraphicsPipelineSpecification& specification)
	{
		return std::make_shared<GraphicsPipeline>(pEngine, pipelineName, pShaders, pRenderTarget, specification);
	}
	
	void GraphicsPipeline::terminate()
	{
		// Destroy the descriptor pool if available. 
		if (m_vDescriptorPool != VK_NULL_HANDLE)
		{
			// Make sure to kill it's kids before killing him ;)
			m_pPackages.clear();
			getEngine()->getDeviceTable().vkDestroyDescriptorPool(getEngine()->getLogicalDevice(), m_vDescriptorPool, nullptr);
		}

		getEngine()->getDeviceTable().vkDestroyPipelineLayout(getEngine()->getLogicalDevice(), m_vPipelineLayout, nullptr);
		getEngine()->getDeviceTable().vkDestroyPipeline(getEngine()->getLogicalDevice(), m_vPipeline, nullptr);
		toggleTerminated();
	}
	
	std::shared_ptr<Package> GraphicsPipeline::createPackage(const Shader* pShader)
	{
		// Check if the shader is within this pipeline.
		const int32_t shaderIndex = getShaderIndex(pShader);
		if (shaderIndex == -1)
			throw BackendError("The provided shader does not exist within the pipeline!");

		// If we don't have bindings to create packages to, lets return a nullptr.
		if (m_DescriptorPoolSizes.empty())
			return nullptr;

		// Setup pool create info.
		VkDescriptorPoolCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		vCreateInfo.flags = 0;
		vCreateInfo.pNext = nullptr;
		vCreateInfo.maxSets = static_cast<uint32_t>(m_pPackages.size()) + 1;
		vCreateInfo.poolSizeCount = static_cast<uint32_t>(m_DescriptorPoolSizes.size());
		vCreateInfo.pPoolSizes = m_DescriptorPoolSizes.data();

		const auto vOldDescriptorPool = m_vDescriptorPool;
		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateDescriptorPool(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vDescriptorPool), "Failed to create the descriptor pool!");

		// Allocate the descriptor sets.
		VkDescriptorSetAllocateInfo vAllocateInfo = {};
		vAllocateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vAllocateInfo.pNext = nullptr;
		vAllocateInfo.descriptorSetCount = 1;
		vAllocateInfo.descriptorPool = m_vDescriptorPool;

		// Allocate new descriptor sets and swap the old ones with them.
		for (const auto& pPackage : m_pPackages)
		{
			const auto layout = pPackage->getDescriptorSetLayout();
			vAllocateInfo.pSetLayouts = &layout;

			// Allocate a new descriptor set.
			VkDescriptorSet vDescriptorSet = VK_NULL_HANDLE;
			FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkAllocateDescriptorSets(getEngine()->getLogicalDevice(), &vAllocateInfo, &vDescriptorSet), "Failed to allocate descriptor set!");

			// Swap the descriptors.
			pPackage->swapDescriptors(m_vDescriptorPool, vDescriptorSet);
		}

		const auto layout = pShader->getDescriptorSetLayout();
		vAllocateInfo.pSetLayouts = &layout;

		// Allocate a new descriptor set.
		VkDescriptorSet vDescriptorSet = VK_NULL_HANDLE;
		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkAllocateDescriptorSets(getEngine()->getLogicalDevice(), &vAllocateInfo, &vDescriptorSet), "Failed to allocate descriptor set!");

		// Create the new package.
		auto pNewPackage = Package::create(std::static_pointer_cast<GraphicsEngine>(getEngine()), layout, m_vDescriptorPool, vDescriptorSet, shaderIndex);
		m_pPackages.emplace_back(pNewPackage);

		getEngine()->getDeviceTable().vkDestroyDescriptorPool(getEngine()->getLogicalDevice(), vOldDescriptorPool, nullptr);
		return pNewPackage;
	}
	
	void GraphicsPipeline::createPipelineLayout()
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

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreatePipelineLayout(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vPipelineLayout), "Failed to create the pipeline layout!");
	}
	
	VkShaderStageFlagBits GraphicsPipeline::getStageFlagBits(const Shader* pShader) const
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
	
	VkFormat GraphicsPipeline::getFormatFromSize(const uint32_t size) const
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
	
	void GraphicsPipeline::createPipeline()
	{
		// Resolve shader info.
		std::vector<VkPipelineShaderStageCreateInfo> vShaderStageCreateInfos;
		vShaderStageCreateInfos.reserve(m_pShaders.size());

		std::vector<VkVertexInputAttributeDescription> vAttributeDescriptions;
		VkVertexInputBindingDescription vBindingDescription = {};

		VkPipelineShaderStageCreateInfo vShaderStageCreateInfo = {};
		vShaderStageCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vShaderStageCreateInfo.pNext = nullptr;
		vShaderStageCreateInfo.flags = 0;
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

			// At the same time, lets also resolve the pool sizes so we don't have to waste a lot of resources later.
			for (const auto& [name, binding] : pShader->getBindings())
			{
				VkDescriptorPoolSize vPoolSize = {};
				vPoolSize.descriptorCount = binding.m_Count;
				vPoolSize.type = binding.m_Type;
				m_DescriptorPoolSizes.emplace_back(vPoolSize);
			}
		}

		// Setup vertex input state.
		VkPipelineVertexInputStateCreateInfo vVertexInputStateCreateInfo = {};
		vVertexInputStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vVertexInputStateCreateInfo.pNext = nullptr;
		vVertexInputStateCreateInfo.flags = 0;
		vVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vAttributeDescriptions.size());
		vVertexInputStateCreateInfo.pVertexAttributeDescriptions = vAttributeDescriptions.data();
		vVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
		vVertexInputStateCreateInfo.pVertexBindingDescriptions = &vBindingDescription;

		// Setup input assembly state.
		VkPipelineInputAssemblyStateCreateInfo vInputAssemblyStateCreateInfo = {};
		vInputAssemblyStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vInputAssemblyStateCreateInfo.pNext = nullptr;
		vInputAssemblyStateCreateInfo.flags = 0;
		vInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
		vInputAssemblyStateCreateInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Setup tessellation state.
		VkPipelineTessellationStateCreateInfo vTessellationStateCreateInfo = {};
		vTessellationStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		vTessellationStateCreateInfo.pNext = nullptr;
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
		vViewportStateCreateInfo.pNext = nullptr;
		vViewportStateCreateInfo.flags = 0;
		vViewportStateCreateInfo.scissorCount = 1;
		vViewportStateCreateInfo.pScissors = &vRect;
		vViewportStateCreateInfo.viewportCount = 1;
		vViewportStateCreateInfo.pViewports = &vViewport;

		// Setup color blend state.
		VkPipelineColorBlendAttachmentState vColorBlendAttachmentState = {};
		vColorBlendAttachmentState.blendEnable = VK_FALSE;
		vColorBlendAttachmentState.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
		vColorBlendAttachmentState.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
		vColorBlendAttachmentState.colorWriteMask =
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT |
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT |
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
		vColorBlendAttachmentState.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
		vColorBlendAttachmentState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
		vColorBlendAttachmentState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
		vColorBlendAttachmentState.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;

		VkPipelineColorBlendStateCreateInfo vColorBlendStateCreateInfo = {};
		vColorBlendStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		vColorBlendStateCreateInfo.pNext = nullptr;
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
		vRasterizationStateCreateInfo.pNext = nullptr;
		vRasterizationStateCreateInfo.flags = 0;
		vRasterizationStateCreateInfo.cullMode = m_Specification.vCullMode;
		vRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
		vRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
		vRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		vRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		vRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		vRasterizationStateCreateInfo.frontFace = m_Specification.vFrontFace;
		vRasterizationStateCreateInfo.lineWidth = 1.0f;
		vRasterizationStateCreateInfo.polygonMode = m_Specification.vPolygonMode;
		vRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;

		// Setup multisample state.
		VkPipelineMultisampleStateCreateInfo vMultisampleStateCreateInfo = {};
		vMultisampleStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		vMultisampleStateCreateInfo.pNext = nullptr;
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
		vDepthStencilStateCreateInfo.pNext = nullptr;
		vDepthStencilStateCreateInfo.flags = 0;
		vDepthStencilStateCreateInfo.back.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
		vDepthStencilStateCreateInfo.front.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER;
		vDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		vDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
		vDepthStencilStateCreateInfo.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;

		// Setup dynamic state.
		std::array<VkDynamicState, 2> vDynamicStates;
		vDynamicStates[0] = VkDynamicState::VK_DYNAMIC_STATE_SCISSOR;
		vDynamicStates[1] = VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT;

		VkPipelineDynamicStateCreateInfo vDynamicStateCreateInfo = {};
		vDynamicStateCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		vDynamicStateCreateInfo.pNext = nullptr;
		vDynamicStateCreateInfo.flags = 0;
		vDynamicStateCreateInfo.dynamicStateCount = 2;
		vDynamicStateCreateInfo.pDynamicStates = vDynamicStates.data();

		// Setup pipeline create info.
		VkGraphicsPipelineCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		vCreateInfo.pNext = nullptr;
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

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateGraphicsPipelines(getEngine()->getLogicalDevice(), m_vPipelineCache, 1, &vCreateInfo, nullptr, &m_vPipeline), "Failed to create the graphics pipeline!");
	}
	
	int32_t GraphicsPipeline::getShaderIndex(const Shader* pShader) const
	{
		// Iterate and see if the shader exists in the pipeline.
		for (int32_t i = 0; i < m_pShaders.size(); i++)
		{
			if (m_pShaders[i].get() == pShader)
				return i;
		}

		return -1;
	}
}