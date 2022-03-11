#pragma once

#include "GraphicsEngine.hpp"
#include "Firefly/Image.hpp"

#include <unordered_map>

namespace Firefly
{
	/**
	 * Package object.
	 * This object is used to submit resources to a pipeline when rendering.
	 *
	 * Note: Make sure that whatever the resource bound to this package lives longer than this object's lifetime.
	 */
	class Package final : EngineBoundObject
	{
		/**
		 * Resource binding structure.
		 * This structure contains information about a single binding and can be used to see if a binding is used or not.
		 */
		struct ResourceBinding
		{
			ResourceBinding() = default;
			explicit ResourceBinding(const std::vector<std::shared_ptr<Buffer>>& pBuffers, const uint32_t arrayElement) : m_pBuffers(pBuffers), m_DestinationArrayElement(arrayElement) {}
			explicit ResourceBinding(const std::vector<std::shared_ptr<Image>>& pImages, const uint32_t arrayElement) : m_pImage(pImages), m_DestinationArrayElement(arrayElement) {}

			bool containsBuffers() const { return !m_pBuffers.empty(); }

			std::vector<std::shared_ptr<Buffer>> m_pBuffers;
			std::vector<std::shared_ptr<Image>> m_pImage;
			uint32_t m_DestinationArrayElement;
		};

	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine to which this object is bound to.
		 * @param vDescriptorSetLayout The descriptor set layout.
		 * @param vDescriptorPool The descriptor pool which the descriptor set is made with.
		 * @param vDescriptorSet The descriptor set used by this package.
		 * @param setIndex The descriptor set index.
		 */
		explicit Package(const std::shared_ptr<GraphicsEngine>& pEngine, const VkDescriptorSetLayout vDescriptorSetLayout, const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet, const uint32_t setIndex);

		/**
		 * Swap the descriptors.
		 * This is done once a new package is made.
		 *
		 * @param vDescriptorPool The new descriptor pool which the descriptor set is made with.
		 * @param vDescriptorSet The new descriptor set.
		 */
		void swapDescriptors(const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet);

		/**
		 * Create a new package.
		 *
		 * @param pEngine The engine to which this object is bound to.
		 * @param vDescriptorSetLayout The descriptor set layout.
		 * @param vDescriptorPool The descriptor pool which the descriptor set is made with.
		 * @param vDescriptorSet The descriptor set used by this package.
		 * @return The created package.
		 */
		static std::shared_ptr<Package> create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkDescriptorSetLayout vDescriptorSetLayout, const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet, const uint32_t setIndex);

		/**
		 * Bind an buffer resources to the package.
		 *
		 * @param binding The binding to which the buffer is bound to.
		 * @param pBuffers The buffer pointers.
		 * @param vDescriptorType The type of the descriptor. Default is Uniform buffer.
		 * @param arrayElement The destination array element to bind the resource to.
		 */
		void bindResources(const uint32_t binding, const std::vector<std::shared_ptr<Buffer>>& pBuffers, const VkDescriptorType vDescriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, const uint32_t arrayElement = 0);

		/**
		 * Bind an image resources to the package.
		 *
		 * @param binding The binding to which the image is bound to.
		 * @param pImapImagesge The image pointers.
		 * @param vDescriptorType The type of the descriptor. Default is Combined Image Sampler.
		 * @param arrayElement The destination array element to bind the resource to.
		 */
		void bindResources(const uint32_t binding, const std::vector<std::shared_ptr<Image>>& pImages, const VkDescriptorType vDescriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, const uint32_t arrayElement = 0);

		/**
		 * Terminate the package.
		 */
		void terminate() override;

		/**
		 * Get the descriptor set layout.
		 *
		 * @return The layout.
		 */
		VkDescriptorSetLayout getDescriptorSetLayout() const { return m_vDescriptorSetLayout; }

		/**
		 * Get the descriptor pool.
		 *
		 * @return The descriptor pool.
		 */
		VkDescriptorPool getDescriptorPool() const { return m_vDescriptorPool; }

		/**
		 * Get the descriptor set.
		 *
		 * @return The descriptor set.
		 */
		VkDescriptorSet getDescriptorSet() const { return m_vDescriptorSet; }

		/**
		 * Get the descriptor set index.
		 *
		 * @return The set index.
		 */
		uint32_t getSetIndex() const { return m_SetIndex; }

	private:
		std::unordered_map<uint32_t, ResourceBinding> m_BindingMap;

		const VkDescriptorSetLayout m_vDescriptorSetLayout;
		VkDescriptorPool m_vDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_vDescriptorSet = VK_NULL_HANDLE;

		const uint32_t m_SetIndex = 0;
	};
}