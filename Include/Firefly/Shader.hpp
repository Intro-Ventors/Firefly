#pragma once

#include "EngineBoundObject.hpp"

#include <filesystem>
#include <unordered_map>

namespace Firefly
{
	/**
	 * Shader binding structure.
	 * This describes a single binding in the shader.
	 */
	struct ShaderBinding
	{
		uint32_t m_Set = 0;
		uint32_t m_Binding = 0;
		uint32_t m_Count = 0;
		VkDescriptorType m_Type = VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
	};

	/**
	 * Shader attribute structure.
	 * Shader attributes are of two types, inputs and outputs.
	 */
	struct ShaderAttribute
	{
		std::string m_Name;
		uint32_t m_Location = 0;
		uint32_t m_Size = 0;
	};

	/**
	 * Shader object.
	 * Shaders are programs that run in the GPU. This object contains one instance of it.
	 */
	class Shader final : public EngineBoundObject
	{
		using ShaderCode = std::vector<uint32_t>;
		using LayoutBindings = std::vector<VkDescriptorSetLayoutBinding>;

	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 * @param path The shader source path.
		 * @param flags The shader stage flags.
		 * @param binding The shader bindings.
		 */
		explicit Shader(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& file, const VkShaderStageFlags flags);

		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 * @param shaderCode The shader source code.
		 * @param flags The shader stage flags.
		 * @param binding The shader bindings.
		 */
		explicit Shader(const std::shared_ptr<Engine>& pEngine, const ShaderCode& shaderCode, const VkShaderStageFlags flags);

		/**
		 * Destructor.
		 */
		~Shader() override;

		/**
		 * Create a new shader object.
		 *
		 * @param pEngine The engine pointer.
		 * @param path The shader source path.
		 * @param flags The shader stage flags.
		 * @param binding The shader bindings.
		 * @return The shader object.
		 */
		static std::shared_ptr<Shader> create(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& file, const VkShaderStageFlags flags);

		/**
		 * Create a new shader object.
		 *
		 * @param pEngine The engine pointer.
		 * @param shaderCode The shader source code.
		 * @param flags The shader stage flags.
		 * @param binding The shader bindings.
		 * @return The shader object.
		 */
		static std::shared_ptr<Shader> create(const std::shared_ptr<Engine>& pEngine, const ShaderCode& shaderCode, const VkShaderStageFlags flags);

		/**
		 * Terminate the shader.
		 */
		void terminate() override;

		/**
		 * Get the shader flags.
		 *
		 * @return The shader flags.
		 */
		VkShaderStageFlags getFlags() const { return m_Flags; }

		/**
		 * Get the shader module.
		 *
		 * @return The shader module.
		 */
		VkShaderModule getShaderModule() const { return m_vShaderModule; }

		/**
		 * Get the descriptor set layout.
		 *
		 * @return The layout.
		 */
		VkDescriptorSetLayout getDescriptorSetLayout() const { return m_vDescriptorSetLayout; }

		/**
		 * Get the bindings.
		 *
		 * @return The binding map.
		 */
		std::unordered_map<std::string, ShaderBinding> getBindings() const { return m_Bindings; }

		/**
		 * Check if a given binding name is present.
		 *
		 * @param name The name of the binding to check.
		 * @return Boolean value stating if its present or not.
		 */
		bool isBindingPresent(const std::string& name) const { return m_Bindings.find(name) != m_Bindings.end(); }

		/**
		 * Get a binding at a given name.
		 *
		 * @param name The name of the binding.
		 * @return The shader binding.
		 */
		ShaderBinding getBinding(const std::string& name) const { return m_Bindings.at(name); }

		/**
		 * Get the shader input attributes.
		 *
		 * @return Input attributes.
		 */
		std::vector<ShaderAttribute> getInputAttributes() const { return m_InputAttributes; }

		/**
		 * Get the shader output attributes.
		 *
		 * @return Output attributes.
		 */
		std::vector<ShaderAttribute> getOutputAttributes() const { return m_OutputAttributes; }

		/**
		 * Get he push constants.
		 *
		 * @return The push constants.
		 */
		std::vector<VkPushConstantRange> getPushConstants() const { return m_PushConstants; }

	private:
		/**
		 * Load the shader code.
		 *
		 * @param file The file path to load the shader from.
		 * @return The shader code.
		 */
		ShaderCode loadCode(const std::filesystem::path& file) const;

		/**
		 * Create the shader module.
		 *
		 * @param code The shader code.
		 */
		void createShaderModule(const ShaderCode& code);

		/**
		 * Perform reflection on the shader code.
		 *
		 * @retrun The layout bindings.
		 */
		LayoutBindings performReflection(const ShaderCode& code);

		/**
		 * Resolve shader code padding.
		 *
		 * @param code The shader code to resolve padding for.
		 * @return The padding resolved code.
		 */
		ShaderCode resolvePadding(const ShaderCode& code) const;

		/**
		 * Validate reflection.
		 * This will check if reflection was successful.
		 *
		 * @param result The reflection result.
		 */
		void validateReflection(SpvReflectResult result) const;

		/**
		 * Get the descriptor type from the reflection descriptor type.
		 *
		 * @return The Vulkan descriptor type.
		 */
		VkDescriptorType getVkDescriptorType(const SpvReflectDescriptorType type) const;

		/**
		 * Create the descriptor set layout binding.
		 *
		 * @param binding The descriptor set layout binding.
		 */
		void createDescriptorSetLayout(const LayoutBindings& bindings);

	private:
		std::unordered_map<std::string, ShaderBinding> m_Bindings;
		std::vector<ShaderAttribute> m_InputAttributes;
		std::vector<ShaderAttribute> m_OutputAttributes;
		std::vector<VkPushConstantRange> m_PushConstants;

		VkShaderModule m_vShaderModule = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_vDescriptorSetLayout = VK_NULL_HANDLE;

		const VkShaderStageFlags m_Flags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
	};
}