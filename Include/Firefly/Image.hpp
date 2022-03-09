#pragma once

#include "EngineBoundObject.hpp"

namespace Firefly
{
	/**
	 * Image type enum.
	 */
	enum class ImageType : uint32_t
	{
		TwoDimension,
		CubeMap,
	};

	/**
	 * Image object.
	 * This object contains a single image which may contain multiple layers.
	 */
	class Image final : public EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer.
		 * @param extent The image extent.
		 * @param format The image format.
		 * @param type The image type.
		 * @param layers The image layers. Default is 1.
		 */
		explicit Image(const std::shared_ptr<Engine>& pEngine, const VkExtent3D extent, const VkFormat format, const ImageType type, const uint32_t layers = 1)
			: EngineBoundObject(pEngine), m_Extent(extent), m_Format(format), m_Type(type), m_Layers(layers)
		{
			// Create the image.
			createImage();

			// Create the image view.
			createImageView();

			// Create the image sampler.
			createSampler();
		}

		/**
		 * Destructor.
		 */
		~Image() override
		{
			if (!isTerminated())
				terminate();
		}

		/**
		 * Terminate the image.
		 */
		void terminate() override
		{
			getEngine()->getDeviceTable().vkDestroyImageView(getEngine()->getLogicalDevice(), m_vImageView, nullptr);
			getEngine()->getDeviceTable().vkDestroySampler(getEngine()->getLogicalDevice(), m_vSampler, nullptr);
			vmaDestroyImage(getEngine()->getAllocator(), m_vImage, m_Allocation);
			toggleTerminated();
		}

		/**
		 * Create a new image object.
		 *
		 * @param pEngine The engine pointer.
		 * @param extent The image extent.
		 * @param format The image format.
		 * @param type The image type.
		 * @param layers The image layers. Default is 1.
		 * @return The image pointer.
		 */
		static std::shared_ptr<Image> create(const std::shared_ptr<Engine>& pEngine, const VkExtent3D extent, const VkFormat format, const ImageType type, const uint32_t layers = 1)
		{
			return std::make_shared<Image>(pEngine, extent, format, type, layers);
		}

		/**
		 * Get the image extent.
		 *
		 * @return The image extent.
		 */
		VkExtent3D getExtent() const { return m_Extent; }

		/**
		 * Get the Vulkan image.
		 *
		 * @return The image.
		 */
		VkImage getImage() const { return m_vImage; }

		/**
		 * Get the image view.
		 *
		 * @return The image view.
		 */
		VkImageView getImageView() const { return m_vImageView; }

		/**
		 * Get the image sampler.
		 *
		 * @return The sampler.
		 */
		VkSampler getSampler() const { return m_vSampler; }

		/**
		 * Get the image format.
		 *
		 * @return The format.
		 */
		VkFormat getFormat() const { return m_Format; }

		/**
		 * Get the image type.
		 *
		 * @return The type.
		 */
		ImageType getType() const { return m_Type; }

		/**
		 * Get the layer count.
		 *
		 * @return The image layer count.
		 */
		uint32_t getLayers() const { return m_Layers; }

	private:
		/**
		 * Create the image.
		 */
		void createImage()
		{
			VkImageCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.extent = m_Extent;
			vCreateInfo.format = m_Format;
			vCreateInfo.arrayLayers = m_Layers;
			vCreateInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			vCreateInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
			vCreateInfo.queueFamilyIndexCount = 0;
			vCreateInfo.pQueueFamilyIndices = nullptr;
			vCreateInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
			vCreateInfo.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
			vCreateInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			vCreateInfo.mipLevels = 1;

			if (m_Type == ImageType::CubeMap)
				vCreateInfo.flags = VkImageCreateFlagBits::VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

			VmaAllocationCreateInfo vAllocationCreateInfo = {};
			vAllocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

			Utility::ValidateResult(vmaCreateImage(getEngine()->getAllocator(), &vCreateInfo, &vAllocationCreateInfo, &m_vImage, &m_Allocation, nullptr), "Failed to create the image!");
		}

		/**
		 * Create the image view.
		 */
		void createImageView()
		{
			VkImageViewCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.flags = 0;
			vCreateInfo.format = m_Format;
			vCreateInfo.image = m_vImage;
			vCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
			vCreateInfo.subresourceRange.layerCount = m_Layers;
			vCreateInfo.subresourceRange.baseArrayLayer = 0;
			vCreateInfo.subresourceRange.levelCount = 1;
			vCreateInfo.subresourceRange.baseMipLevel = 0;
			vCreateInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			vCreateInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
			vCreateInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
			vCreateInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
			vCreateInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;

			if (m_Type == ImageType::CubeMap)
				vCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateImageView(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vImageView), "Failed to create the image view!");
		}

		/**
		 * Create image sampler.
		 */
		void createSampler()
		{
			VkSamplerAddressMode addressMode = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			if (m_Type == ImageType::CubeMap)	// TODO
				addressMode = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			VkSamplerCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			vCreateInfo.pNext = nullptr;
			vCreateInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
			vCreateInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
			vCreateInfo.addressModeU = addressMode;
			vCreateInfo.addressModeV = addressMode;
			vCreateInfo.addressModeW = addressMode;
			vCreateInfo.anisotropyEnable = VK_TRUE;
			vCreateInfo.maxAnisotropy = getEngine()->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
			vCreateInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			vCreateInfo.unnormalizedCoordinates = VK_FALSE;
			vCreateInfo.compareEnable = VK_FALSE;
			vCreateInfo.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
			vCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
			vCreateInfo.minLod = 0.0;
			vCreateInfo.maxLod = 1;
			vCreateInfo.mipLodBias = 0.0;

			Utility::ValidateResult(getEngine()->getDeviceTable().vkCreateSampler(getEngine()->getLogicalDevice(), &vCreateInfo, nullptr, &m_vSampler), "Failed to create the image sampler!");
		}

	private:
		VkExtent3D m_Extent;

		VkImage m_vImage = VK_NULL_HANDLE;
		VkImageView m_vImageView = VK_NULL_HANDLE;
		VkSampler m_vSampler = VK_NULL_HANDLE;

		VmaAllocation m_Allocation = nullptr;

		VkFormat m_Format = VkFormat::VK_FORMAT_UNDEFINED;
		ImageType m_Type = ImageType::TwoDimension;
		uint32_t m_Layers = 0;
	};
}