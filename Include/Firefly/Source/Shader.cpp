#include "Firefly/Shader.hpp"

#include <fstream>

namespace Firefly
{
	Shader::Shader(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& file, const VkShaderStageFlags flags)
		: EngineBoundObject(pEngine), m_Flags(flags)
	{
		// Load the shader data.
		const auto shaderCode = loadCode(file);

		// Create the shader module.
		createShaderModule(shaderCode);

		// Perform reflection.
		const auto bindings = performReflection(shaderCode);

		// Create the descriptor set layout.
		createDescriptorSetLayout(bindings);
	}
	
	Shader::Shader(const std::shared_ptr<Engine>& pEngine, const ShaderCode& shaderCode, const VkShaderStageFlags flags)
		: EngineBoundObject(pEngine), m_Flags(flags)
	{
		// Create the shader module.
		createShaderModule(shaderCode);

		// Perform reflection.
		const auto bindings = performReflection(shaderCode);

		// Create the descriptor set layout.
		createDescriptorSetLayout(bindings);
	}
	
	Shader::~Shader()
	{
		if (!isTerminated())
			terminate();
	}
	
	std::shared_ptr<Shader> Shader::create(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& file, const VkShaderStageFlags flags)
	{
		return std::make_shared<Shader>(pEngine, file, flags);
	}
	
	std::shared_ptr<Shader> Shader::create(const std::shared_ptr<Engine>& pEngine, const ShaderCode& shaderCode, const VkShaderStageFlags flags)
	{
		return std::make_shared<Shader>(pEngine, shaderCode, flags);
	}
	
	void Shader::terminate()
	{
		getEngine()->getDeviceTable().vkDestroyDescriptorSetLayout(getEngine()->getLogicalDevice(), m_vDescriptorSetLayout, nullptr);
		getEngine()->getDeviceTable().vkDestroyShaderModule(getEngine()->getLogicalDevice(), m_vShaderModule, nullptr);
		toggleTerminated();
	}
	
	Shader::ShaderCode Shader::loadCode(const std::filesystem::path& file) const
	{
		// Load the shader file.
		std::fstream shaderFile(file, std::ios::in | std::ios::binary | std::ios::ate);

		if (!shaderFile.is_open())
			throw BackendError("Could not open the file specified!");

		// Get the file size.
		const auto size = shaderFile.tellg();
		shaderFile.seekg(0);

		// Load its content.
		ShaderCode shaderCode(size);
		shaderFile.read(reinterpret_cast<char*>(shaderCode.data()), size);
		shaderFile.close();

		return shaderCode;
	}
	
	void Shader::createShaderModule(const ShaderCode& code)
	{
		VkShaderModuleCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vCreateInfo.pNext = VK_NULL_HANDLE;
		vCreateInfo.flags = 0;
		vCreateInfo.codeSize = code.size();
		vCreateInfo.pCode = code.data();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateShaderModule(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vShaderModule), "Failed to create the shader module!");
	}
	
	Shader::LayoutBindings Shader::performReflection(const ShaderCode& code)
	{
		SpvReflectShaderModule shaderModule = {};
		uint32_t variableCount = 0;

		const ShaderCode shaderCode = resolvePadding(code);
		validateReflection(spvReflectCreateShaderModule(shaderCode.size() * sizeof(uint32_t), shaderCode.data(), &shaderModule));

		// Resolve shader inputs.
		{
			validateReflection(spvReflectEnumerateInputVariables(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectInterfaceVariable*> pInputs(variableCount);
			validateReflection(spvReflectEnumerateInputVariables(&shaderModule, &variableCount, pInputs.data()));

			m_InputAttributes.reserve(pInputs.size());

			// Iterate through the attributes and load them.
			for (auto& resource : pInputs)
			{
				if (resource->format == SpvReflectFormat::SPV_REFLECT_FORMAT_UNDEFINED)
					continue;

				// If the type is not a built in one, we can proceed to add them.
				if (resource->built_in == -1)
				{
					ShaderAttribute attribute;

					if (resource->name)
						attribute.m_Name = resource->name;

					attribute.m_Location = resource->location;
					attribute.m_Size = (resource->type_description->traits.numeric.scalar.width / 8) *
						std::max(resource->type_description->traits.numeric.vector.component_count, uint32_t(1));

					m_InputAttributes.emplace_back(attribute);
				}
			}
		}

		// Sort the inputs.
		std::sort(m_InputAttributes.begin(), m_InputAttributes.end(), [](const ShaderAttribute& lhs, const ShaderAttribute& rhs) { return lhs.m_Location < rhs.m_Location; });

		// Resolve shader outputs.
		{
			validateReflection(spvReflectEnumerateOutputVariables(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectInterfaceVariable*> pOutputs(variableCount);
			validateReflection(spvReflectEnumerateOutputVariables(&shaderModule, &variableCount, pOutputs.data()));

			m_OutputAttributes.reserve(pOutputs.size());

			// Iterate through the attributes and load them.
			for (auto& resource : pOutputs)
			{
				if (resource->format == SpvReflectFormat::SPV_REFLECT_FORMAT_UNDEFINED)
					continue;

				// If the type is not a built in one, we can proceed to add them.
				if (resource->built_in == -1)
				{
					ShaderAttribute attribute;

					if (resource->name)
						attribute.m_Name = resource->name;

					attribute.m_Location = resource->location;
					attribute.m_Size = (resource->type_description->traits.numeric.scalar.width / 8) *
						std::max(resource->type_description->traits.numeric.vector.component_count, uint32_t(1));

					m_OutputAttributes.emplace_back(attribute);
				}
			}
		}

		// Sort the outputs.
		std::sort(m_OutputAttributes.begin(), m_OutputAttributes.end(), [](const ShaderAttribute& lhs, const ShaderAttribute& rhs) { return lhs.m_Location < rhs.m_Location; });

		LayoutBindings bindings;

		// Resolve uniforms.
		{
			validateReflection(spvReflectEnumerateDescriptorBindings(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectDescriptorBinding*> pBindings(variableCount);
			validateReflection(spvReflectEnumerateDescriptorBindings(&shaderModule, &variableCount, pBindings.data()));

			bindings.reserve(variableCount);

			VkDescriptorSetLayoutBinding vBinding = {};
			vBinding.stageFlags = m_Flags;
			vBinding.pImmutableSamplers = VK_NULL_HANDLE;

			// Iterate over the resources and setup the bindings.
			for (auto& resource : pBindings)
			{
				vBinding.descriptorType = getVkDescriptorType(resource->descriptor_type);
				vBinding.descriptorCount = resource->count;
				vBinding.binding = resource->binding;

				//mDescriptorSetMap[resource->set].mLayoutBindings.emplace_back(vBinding);
				//mDescriptorSetMap[resource->set].mPoolSizes.emplace_back(vSize);
				//
				//ShaderResourceKey key{ resource->set,vBinding.binding };
				//mResourceMap[resource->set][resource->binding] = Helpers::GetShaderResourceType(resource->descriptor_type);

				ShaderBinding binding;
				binding.m_Binding = resource->binding;
				binding.m_Set = resource->set;
				binding.m_Count = resource->count;
				binding.m_Type = vBinding.descriptorType;

				m_Bindings[resource->name] = binding;
				bindings.emplace_back(vBinding);
			}
		}

		// Resolve push constants.
		{
			validateReflection(spvReflectEnumeratePushConstantBlocks(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectBlockVariable*> pPushConstants(variableCount);
			validateReflection(spvReflectEnumeratePushConstantBlocks(&shaderModule, &variableCount, pPushConstants.data()));

			VkPushConstantRange vPushConstantRange = {};
			vPushConstantRange.stageFlags = m_Flags;
			vPushConstantRange.offset = 0;

			// Iterate over the push constants and setup.
			for (auto& resource : pPushConstants)
			{
				vPushConstantRange.size = resource->size;
				vPushConstantRange.offset = resource->offset;
				m_PushConstants.emplace_back(vPushConstantRange);
			}
		}

		return bindings;
	}
	
	Shader::ShaderCode Shader::resolvePadding(const ShaderCode& code) const
	{
		const auto finalCodeSize = code.size() / 4;
		ShaderCode resolvedCode(code.begin(), code.begin() + finalCodeSize);

		return resolvedCode;
	}

	void Shader::validateReflection(SpvReflectResult result) const
	{
		switch (result)
		{
		case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:
			throw BackendError("Shader parse failed!");

		case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:
			throw BackendError("Shader allocation failed!");

		case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:
			throw BackendError("Shader range exceeded!");

		case SPV_REFLECT_RESULT_ERROR_NULL_POINTER:
			throw BackendError("Shader null pointer!");

		case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:
			throw BackendError("Shader internal reflection error!");

		case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:
			throw BackendError("Shader count mismatch!");

		case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:
			throw BackendError("Shader element not found!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:
			throw BackendError("Shader invalid SPIRV code size!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:
			throw BackendError("Shader invalid SPIRV magic number!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:
			throw BackendError("Shader SPIRV unexpected end of file (EOF)!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:
			throw BackendError("Shader invalid SPIRV ID reference!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:
			throw BackendError("Shader invalid SPIRV descriptor set number overflow!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:
			throw BackendError("Shader invalid SPIRV storage class!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:
			throw BackendError("Shader invalid SPIRV recursion!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:
			throw BackendError("Shader invalid SPIRV instruction!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:
			throw BackendError("Shader invalid SPIRV block data!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:
			throw BackendError("Shader invalid SPIRV block member reference!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:
			throw BackendError("Shader invalid SPIRV entry point!");

		case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:
			throw BackendError("Shader invalid SPIRV execution mode!");
		}
	}

	VkDescriptorType Shader::getVkDescriptorType(const SpvReflectDescriptorType type) const
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		default:
			throw BackendError("Invalid shader descriptor type!");
		}

		return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
	
	void Shader::createDescriptorSetLayout(const LayoutBindings& bindings)
	{
		VkDescriptorSetLayoutCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vCreateInfo.pNext = nullptr;
		vCreateInfo.flags = 0;
		vCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		vCreateInfo.pBindings = bindings.data();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateDescriptorSetLayout(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vDescriptorSetLayout), "Failed to create descriptor set layout!");
	}
}