#pragma once

#include "RenderTarget.hpp"
#include "Firefly/Shader.hpp"
#include "Package.hpp"

namespace Firefly
{
	/**
	 * Graphics pipeline specification structure.
	 * This contains a few information which would be needed when creating the pipeline.
	 */
	struct GraphicsPipelineSpecification
	{
		VkCullModeFlags vCullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
		VkFrontFace vFrontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
		VkPolygonMode vPolygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
	};

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
		 * @param specification The pipeline specification.
		 */
		explicit GraphicsPipeline(const std::shared_ptr<GraphicsEngine>& pEngine, const std::string& pipelineName, const std::vector<std::shared_ptr<Shader>>& pShaders,
			const std::shared_ptr<RenderTarget>& pRenderTarget, const GraphicsPipelineSpecification& specification = GraphicsPipelineSpecification());

		/**
		 * Destructor.
		 */
		~GraphicsPipeline() override;

		/**
		 * Create a new graphics pipeline.
		 *
		 * @param pEngine The engine pointer.
		 * @param pipelineName The unique name given to the pipeline. This will be used for caching.
		 * @param pShaders The shaders used by the pipeline.
		 * @param pRenderTarget The render target pointer to which this pipeline is bound to.
		 * @param specification The pipeline specification.
		 * @return The graphics pipeline.
		 */
		static std::shared_ptr<GraphicsPipeline> create(const std::shared_ptr<GraphicsEngine>& pEngine, const std::string& pipelineName, const std::vector<std::shared_ptr<Shader>>& pShaders,
			const std::shared_ptr<RenderTarget>& pRenderTarget, const GraphicsPipelineSpecification& specification = GraphicsPipelineSpecification());

		/**
		 * Terminate the pipeline.
		 */
		void terminate() override;

		/**
		 * Create a new package.
		 *
		 * @param pShader The shader to which the package is bound to.
		 * @return The created package.
		 */
		std::shared_ptr<Package> createPackage(const Shader* pShader);

		/**
		 * Get the pipeline layout.
		 *
		 * @return The Vulkan pipeline layout.
		 */
		VkPipelineLayout getPipelineLayout() const { return m_vPipelineLayout; }

		/**
		 * Get the pipeline.
		 *
		 * @return The Vulkan pipeline.
		 */
		VkPipeline getPipeline() const { return m_vPipeline; }

	private:
		/**
		 * Check if a shader exists in the pipeline.
		 *
		 * @param pShader The shader to check.
		 * @return Index of the shader.
		 */
		int32_t getShaderIndex(const Shader* pShader) const;

	private:
		const std::string m_Name;
		const std::vector<std::shared_ptr<Shader>> m_pShaders;
		std::vector<VkDescriptorPoolSize> m_DescriptorPoolSizes;
		std::vector<std::shared_ptr<Package>> m_pPackages;

		const std::shared_ptr<RenderTarget> m_pRenderTarget = nullptr;

		VkPipelineLayout m_vPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_vPipeline = VK_NULL_HANDLE;
		VkPipelineCache m_vPipelineCache = VK_NULL_HANDLE;

		VkDescriptorPool m_vDescriptorPool = VK_NULL_HANDLE;

		GraphicsPipelineSpecification m_Specification = {};
	};
}