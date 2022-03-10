#pragma once

#include "GraphicsEngine.hpp"
#include "Firefly/Image.hpp"

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
		explicit Package(const std::shared_ptr<GraphicsEngine>& pEngine, const VkDescriptorSetLayout vDescriptorSetLayout, const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet, const uint32_t setIndex)
			: EngineBoundObject(pEngine), m_vDescriptorSetLayout(vDescriptorSetLayout), m_vDescriptorPool(vDescriptorPool), m_vDescriptorSet(vDescriptorSet), m_SetIndex(setIndex)
		{
		}

		/**
		 * Swap the descriptors.
		 * This is done once a new package is made.
		 *
		 * @param vDescriptorPool The new descriptor pool which the descriptor set is made with.
		 * @param vDescriptorSet The new descriptor set.
		 */
		void swapDescriptors(const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet)
		{
			std::vector<VkCopyDescriptorSet> vCopies;
			vCopies.reserve(m_BindingMap.size());

			VkCopyDescriptorSet vCopyInfo = {};
			vCopyInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
			vCopyInfo.pNext = nullptr;
			vCopyInfo.srcSet = m_vDescriptorSet;
			vCopyInfo.dstSet = vDescriptorSet;

			// Iterate through the bindings and setup the copy info.
			for (const auto& [binding, resource] : m_BindingMap)
			{
				// If it only contains buffers, we just need to get the buffer count.
				if (resource.containsBuffers())
					vCopyInfo.descriptorCount = static_cast<uint32_t>(resource.m_pBuffers.size());

				// Else, we get the image count.
				else
					vCopyInfo.descriptorCount = static_cast<uint32_t>(resource.m_pImage.size());

				vCopyInfo.dstBinding = binding;
				vCopyInfo.srcBinding = binding;
				vCopyInfo.dstArrayElement = resource.m_DestinationArrayElement;
				vCopyInfo.srcArrayElement = resource.m_DestinationArrayElement;

				vCopies.emplace_back(vCopyInfo);
			}

			// Copy everything to the new descriptor set.
			getEngine()->getDeviceTable().vkUpdateDescriptorSets(getEngine()->getLogicalDevice(), 0, nullptr, static_cast<uint32_t>(vCopies.size()), vCopies.data());

			m_vDescriptorSet = vDescriptorSet;
			m_vDescriptorPool = vDescriptorPool;
		}

		/**
		 * Create a new package.
		 *
		 * @param pEngine The engine to which this object is bound to.
		 * @param vDescriptorSetLayout The descriptor set layout.
		 * @param vDescriptorPool The descriptor pool which the descriptor set is made with.
		 * @param vDescriptorSet The descriptor set used by this package.
		 * @return The created package.
		 */
		static std::shared_ptr<Package> create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkDescriptorSetLayout vDescriptorSetLayout, const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet, const uint32_t setIndex)
		{
			return std::make_shared<Package>(pEngine, vDescriptorSetLayout, vDescriptorPool, vDescriptorSet, setIndex);
		}

		/**
		 * Bind an buffer resources to the package.
		 *
		 * @param binding The binding to which the buffer is bound to.
		 * @param pBuffers The buffer pointers.
		 * @param arrayElement The destination array element to bind the resource to.
		 */
		void bindResources(const uint32_t binding, const std::vector<std::shared_ptr<Buffer>>& pBuffers, const uint32_t arrayElement = 0)
		{
			VkWriteDescriptorSet vWrite = {};
			vWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			vWrite.pNext = nullptr;
			vWrite.pImageInfo = nullptr;
			vWrite.pTexelBufferView = nullptr;
			vWrite.dstSet = m_vDescriptorSet;
			vWrite.descriptorCount = static_cast<uint32_t>(pBuffers.size());
			vWrite.dstArrayElement = arrayElement;
			vWrite.dstBinding = binding;

			// Iterate over the buffers and setup info.
			std::vector<VkDescriptorBufferInfo> vBufferInfos(pBuffers.size());
			for (uint64_t i = 0; i < pBuffers.size(); i++)
			{
				const auto& pBuffer = pBuffers[i];
				vBufferInfos[i].offset = 0;	// TODO
				vBufferInfos[i].range = pBuffer->size();
				vBufferInfos[i].buffer = pBuffer->getBuffer();
			}

			vWrite.pBufferInfo = vBufferInfos.data();
			getEngine()->getDeviceTable().vkUpdateDescriptorSets(getEngine()->getLogicalDevice(), 1, &vWrite, 0, nullptr);

			m_BindingMap[binding] = ResourceBinding(pBuffers, arrayElement);
		}

		/**
		 * Bind an image resources to the package.
		 *
		 * @param binding The binding to which the image is bound to.
		 * @param pImapImagesge The image pointers.
		 * @param arrayElement The destination array element to bind the resource to.
		 */
		void bindResources(const uint32_t binding, const std::vector<std::shared_ptr<Image>>& pImages, const uint32_t arrayElement = 0)
		{
			VkWriteDescriptorSet vWrite = {};
			vWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			vWrite.pNext = nullptr;
			vWrite.pBufferInfo = nullptr;
			vWrite.pTexelBufferView = nullptr;
			vWrite.dstSet = m_vDescriptorSet;
			vWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			vWrite.descriptorCount = static_cast<uint32_t>(pImages.size());
			vWrite.dstArrayElement = arrayElement;
			vWrite.dstBinding = binding;

			// Iterate over the images and setup info.
			std::vector<VkDescriptorImageInfo> vImageInfos(pImages.size());
			for (uint64_t i = 0; i < pImages.size(); i++)
			{
				const auto& pImage = pImages[i];
				vImageInfos[i].sampler = pImage->getSampler();
				vImageInfos[i].imageView = pImage->getImageView();
				vImageInfos[i].imageLayout = pImage->getImageLayout();
			}

			vWrite.pImageInfo = vImageInfos.data();
			getEngine()->getDeviceTable().vkUpdateDescriptorSets(getEngine()->getLogicalDevice(), 1, &vWrite, 0, nullptr);

			m_BindingMap[binding] = ResourceBinding(pImages, arrayElement);
		}

		/**
		 * Terminate the package.
		 */
		void terminate() override { toggleTerminated(); }

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