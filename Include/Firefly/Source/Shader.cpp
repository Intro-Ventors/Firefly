#include "Firefly/Shader.hpp"

#include <fstream>

#ifdef max
#undef max

#endif

namespace /* anonymous */
{
	Firefly::Shader::ShaderCode LoadCode(const std::filesystem::path& file)
	{
		// Load the shader file.
		std::fstream shaderFile(file, std::ios::in | std::ios::binary | std::ios::ate);

		if (!shaderFile.is_open())
			throw Firefly::BackendError("Could not open the file specified!");

		// Get the file size.
		const auto size = shaderFile.tellg();
		shaderFile.seekg(0);

		// Load its content.
		Firefly::Shader::ShaderCode shaderCode(size);
		shaderFile.read(reinterpret_cast<char*>(shaderCode.data()), size);
		shaderFile.close();

		return shaderCode;
	}

	Firefly::Shader::ShaderCode ResolvePadding(const Firefly::Shader::ShaderCode& code)
	{
		return Firefly::Shader::ShaderCode(code.begin(), code.begin() + (code.size() / 4));
	}

	void ValidateReflection(const SpvReflectResult result)
	{
		switch (result)
		{
		case SpvReflectResult::SPV_REFLECT_RESULT_SUCCESS:											return;
		case SpvReflectResult::SPV_REFLECT_RESULT_NOT_READY:										throw Firefly::BackendError("Shader not ready!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:								throw Firefly::BackendError("Shader parse failed!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:								throw Firefly::BackendError("Shader allocation failed!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:								throw Firefly::BackendError("Shader range exceeded!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_NULL_POINTER:								throw Firefly::BackendError("Shader null pointer!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:								throw Firefly::BackendError("Shader internal reflection error!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:								throw Firefly::BackendError("Shader count mismatch!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:							throw Firefly::BackendError("Shader element not found!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:					throw Firefly::BackendError("Shader invalid SPIRV code size!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:					throw Firefly::BackendError("Shader invalid SPIRV magic number!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:						throw Firefly::BackendError("Shader SPIRV unexpected end of file (EOF)!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:					throw Firefly::BackendError("Shader invalid SPIRV ID reference!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:					throw Firefly::BackendError("Shader invalid SPIRV descriptor set number overflow!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:				throw Firefly::BackendError("Shader invalid SPIRV storage class!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:							throw Firefly::BackendError("Shader invalid SPIRV recursion!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:					throw Firefly::BackendError("Shader invalid SPIRV instruction!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:				throw Firefly::BackendError("Shader invalid SPIRV block data!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:		throw Firefly::BackendError("Shader invalid SPIRV block member reference!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:					throw Firefly::BackendError("Shader invalid SPIRV entry point!");
		case SpvReflectResult::SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:				throw Firefly::BackendError("Shader invalid SPIRV execution mode!");
		default:																					throw Firefly::BackendError("Unknown reflection error!");
		}
	}

	VkDescriptorType GetVkDescriptorType(const SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:							return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:			return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:					return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:					return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:					return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:					return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:				return VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		default:																					throw Firefly::BackendError("Invalid shader descriptor type!");
		}
	}

	struct ReflectionResult
	{
		Firefly::Shader::LayoutBindings m_LayoutBindings;
		std::unordered_map<std::string, Firefly::ShaderBinding> m_Bindings;
		std::vector<Firefly::ShaderAttribute> m_InputAttributes;
		std::vector<Firefly::ShaderAttribute> m_OutputAttributes;
		std::vector<VkPushConstantRange> m_PushConstants;

	};

	ReflectionResult PerformReflection(const Firefly::Shader::ShaderCode& code, const VkShaderStageFlags flags)
	{
		SpvReflectShaderModule shaderModule = {};
		uint32_t variableCount = 0;
		ReflectionResult result;

		const Firefly::Shader::ShaderCode shaderCode = ResolvePadding(code);
		ValidateReflection(spvReflectCreateShaderModule(shaderCode.size() * sizeof(uint32_t), shaderCode.data(), &shaderModule));

		// Resolve shader inputs.
		{
			ValidateReflection(spvReflectEnumerateInputVariables(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectInterfaceVariable*> pInputs(variableCount);
			ValidateReflection(spvReflectEnumerateInputVariables(&shaderModule, &variableCount, pInputs.data()));

			result.m_InputAttributes.reserve(pInputs.size());

			// Iterate through the attributes and load them.
			for (auto& resource : pInputs)
			{
				if (resource->format == SpvReflectFormat::SPV_REFLECT_FORMAT_UNDEFINED)
					continue;

				// If the type is not a built in one, we can proceed to add them.
				if (resource->built_in == -1)
				{
					Firefly::ShaderAttribute attribute;

					if (resource->name)
						attribute.m_Name = resource->name;

					attribute.m_Location = resource->location;
					attribute.m_Size = (resource->type_description->traits.numeric.scalar.width / 8) *
						std::max(resource->type_description->traits.numeric.vector.component_count, uint32_t(1));

					result.m_InputAttributes.emplace_back(attribute);
				}
			}
		}

		// Sort the inputs.
		std::sort(result.m_InputAttributes.begin(), result.m_InputAttributes.end(), [](const Firefly::ShaderAttribute& lhs, const Firefly::ShaderAttribute& rhs) { return lhs.m_Location < rhs.m_Location; });

		// Resolve shader outputs.
		{
			ValidateReflection(spvReflectEnumerateOutputVariables(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectInterfaceVariable*> pOutputs(variableCount);
			ValidateReflection(spvReflectEnumerateOutputVariables(&shaderModule, &variableCount, pOutputs.data()));

			result.m_OutputAttributes.reserve(pOutputs.size());

			// Iterate through the attributes and load them.
			for (auto& resource : pOutputs)
			{
				if (resource->format == SpvReflectFormat::SPV_REFLECT_FORMAT_UNDEFINED)
					continue;

				// If the type is not a built in one, we can proceed to add them.
				if (resource->built_in == -1)
				{
					Firefly::ShaderAttribute attribute;

					if (resource->name)
						attribute.m_Name = resource->name;

					attribute.m_Location = resource->location;
					attribute.m_Size = (resource->type_description->traits.numeric.scalar.width / 8) *
						std::max(resource->type_description->traits.numeric.vector.component_count, uint32_t(1));

					result.m_OutputAttributes.emplace_back(attribute);
				}
			}
		}

		// Sort the outputs.
		std::sort(result.m_OutputAttributes.begin(), result.m_OutputAttributes.end(), [](const Firefly::ShaderAttribute& lhs, const Firefly::ShaderAttribute& rhs) { return lhs.m_Location < rhs.m_Location; });

		// Resolve uniforms.
		{
			ValidateReflection(spvReflectEnumerateDescriptorBindings(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectDescriptorBinding*> pBindings(variableCount);
			ValidateReflection(spvReflectEnumerateDescriptorBindings(&shaderModule, &variableCount, pBindings.data()));

			result.m_LayoutBindings.reserve(variableCount);

			VkDescriptorSetLayoutBinding vBinding = {};
			vBinding.stageFlags = flags;
			vBinding.pImmutableSamplers = VK_NULL_HANDLE;

			// Iterate over the resources and setup the bindings.
			for (auto& resource : pBindings)
			{
				vBinding.descriptorType = GetVkDescriptorType(resource->descriptor_type);
				vBinding.descriptorCount = resource->count;
				vBinding.binding = resource->binding;

				Firefly::ShaderBinding binding;
				binding.m_Binding = resource->binding;
				binding.m_Set = resource->set;
				binding.m_Count = resource->count;
				binding.m_Type = vBinding.descriptorType;

				result.m_Bindings[resource->name] = binding;
				result.m_LayoutBindings.emplace_back(vBinding);
			}
		}

		// Resolve push constants.
		{
			ValidateReflection(spvReflectEnumeratePushConstantBlocks(&shaderModule, &variableCount, nullptr));

			std::vector<SpvReflectBlockVariable*> pPushConstants(variableCount);
			ValidateReflection(spvReflectEnumeratePushConstantBlocks(&shaderModule, &variableCount, pPushConstants.data()));

			VkPushConstantRange vPushConstantRange = {};
			vPushConstantRange.stageFlags = flags;
			vPushConstantRange.offset = 0;

			// Iterate over the push constants and setup.
			for (auto& resource : pPushConstants)
			{
				vPushConstantRange.size = resource->size;
				vPushConstantRange.offset = resource->offset;
				result.m_PushConstants.emplace_back(vPushConstantRange);
			}
		}

		return result;
	}
}

namespace Firefly
{
	Shader::Shader(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& file, const VkShaderStageFlags flags)
		: EngineBoundObject(pEngine), m_Flags(flags)
	{
		// Load the shader data.
		const auto shaderCode = LoadCode(file);

		// Create the shader module.
		VkShaderModuleCreateInfo vShaderModuleCreateInfo = {};
		vShaderModuleCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vShaderModuleCreateInfo.pNext = VK_NULL_HANDLE;
		vShaderModuleCreateInfo.flags = 0;
		vShaderModuleCreateInfo.codeSize = shaderCode.size();
		vShaderModuleCreateInfo.pCode = shaderCode.data();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateShaderModule(getEngine()->getLogicalDevice(), &vShaderModuleCreateInfo, nullptr, &m_vShaderModule), "Failed to create the shader module!");

		// Perform reflection.
		auto result = PerformReflection(shaderCode, flags);
		m_InputAttributes = std::move(result.m_InputAttributes);
		m_OutputAttributes = std::move(result.m_OutputAttributes);
		m_Bindings = std::move(result.m_Bindings);
		m_PushConstants = std::move(result.m_PushConstants);

		// Create the descriptor set layout.
		VkDescriptorSetLayoutCreateInfo vDescriptorSetLayoutCreateInfo = {};
		vDescriptorSetLayoutCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vDescriptorSetLayoutCreateInfo.pNext = nullptr;
		vDescriptorSetLayoutCreateInfo.flags = 0;
		vDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(result.m_LayoutBindings.size());
		vDescriptorSetLayoutCreateInfo.pBindings = result.m_LayoutBindings.data();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateDescriptorSetLayout(getEngine()->getLogicalDevice(), &vDescriptorSetLayoutCreateInfo, nullptr, &m_vDescriptorSetLayout), "Failed to create descriptor set layout!");
	}
	
	Shader::Shader(const std::shared_ptr<Engine>& pEngine, const ShaderCode& shaderCode, const VkShaderStageFlags flags)
		: EngineBoundObject(pEngine), m_Flags(flags)
	{
		// Create the shader module.
		VkShaderModuleCreateInfo vShaderModuleCreateInfo = {};
		vShaderModuleCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vShaderModuleCreateInfo.pNext = VK_NULL_HANDLE;
		vShaderModuleCreateInfo.flags = 0;
		vShaderModuleCreateInfo.codeSize = shaderCode.size();
		vShaderModuleCreateInfo.pCode = shaderCode.data();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateShaderModule(getEngine()->getLogicalDevice(), &vShaderModuleCreateInfo, nullptr, &m_vShaderModule), "Failed to create the shader module!");

		// Perform reflection.
		auto result = PerformReflection(shaderCode, flags);
		m_InputAttributes = std::move(result.m_InputAttributes);
		m_OutputAttributes = std::move(result.m_OutputAttributes);
		m_Bindings = std::move(result.m_Bindings);
		m_PushConstants = std::move(result.m_PushConstants);

		// Create the descriptor set layout.
		VkDescriptorSetLayoutCreateInfo vDescriptorSetLayoutCreateInfo = {};
		vDescriptorSetLayoutCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vDescriptorSetLayoutCreateInfo.pNext = nullptr;
		vDescriptorSetLayoutCreateInfo.flags = 0;
		vDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(result.m_LayoutBindings.size());
		vDescriptorSetLayoutCreateInfo.pBindings = result.m_LayoutBindings.data();

		FIREFLY_VALIDATE(getEngine()->getDeviceTable().vkCreateDescriptorSetLayout(getEngine()->getLogicalDevice(), &vDescriptorSetLayoutCreateInfo, nullptr, &m_vDescriptorSetLayout), "Failed to create descriptor set layout!");
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
}