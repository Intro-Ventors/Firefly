#include "Firefly/Graphics/Package.hpp"

namespace Firefly
{
	Package::Package(const std::shared_ptr<GraphicsEngine>& pEngine, const VkDescriptorSetLayout vDescriptorSetLayout, const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet, const uint32_t setIndex)
		: EngineBoundObject(pEngine), m_vDescriptorSetLayout(vDescriptorSetLayout), m_vDescriptorPool(vDescriptorPool), m_vDescriptorSet(vDescriptorSet), m_SetIndex(setIndex)
	{
	}

	void Package::swapDescriptors(const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet)
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

	std::shared_ptr<Package> Package::create(const std::shared_ptr<GraphicsEngine>& pEngine, const VkDescriptorSetLayout vDescriptorSetLayout, const VkDescriptorPool vDescriptorPool, const VkDescriptorSet vDescriptorSet, const uint32_t setIndex)
	{
		return std::make_shared<Package>(pEngine, vDescriptorSetLayout, vDescriptorPool, vDescriptorSet, setIndex);
	}

	void Package::bindResources(const uint32_t binding, const std::vector<std::shared_ptr<Buffer>>& pBuffers, const VkDescriptorType vDescriptorType, const uint32_t arrayElement)
	{
		VkWriteDescriptorSet vWrite = {};
		vWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vWrite.pNext = nullptr;
		vWrite.pImageInfo = nullptr;
		vWrite.pTexelBufferView = nullptr;
		vWrite.dstSet = m_vDescriptorSet;
		vWrite.descriptorType = vDescriptorType;
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

	void Package::bindResources(const uint32_t binding, const std::vector<std::shared_ptr<Image>>& pImages, const VkDescriptorType vDescriptorType, const uint32_t arrayElement)
	{
		VkWriteDescriptorSet vWrite = {};
		vWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vWrite.pNext = nullptr;
		vWrite.pBufferInfo = nullptr;
		vWrite.pTexelBufferView = nullptr;
		vWrite.dstSet = m_vDescriptorSet;
		vWrite.descriptorType = vDescriptorType;
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
	
	void Package::terminate()
	{
		toggleTerminated();
	}
}