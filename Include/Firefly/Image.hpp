#pragma once

#include "Buffer.hpp"

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
		 * @param usageFlags The image usage flags. Default is sampled | transfer source | transfer destination.
		 */
		explicit Image(const std::shared_ptr<Engine>& pEngine, const VkExtent3D extent, const VkFormat format, const ImageType type, const uint32_t layers = 1,
			const VkImageUsageFlags usageFlags = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			: EngineBoundObject(pEngine), m_Extent(extent), m_Format(format), m_Type(type), m_Layers(layers), m_UsageFlags(usageFlags)
		{
			// Create the image.
			createImage();

			// Create the image view.
			createImageView();

			// Resolve the sampler usage.
			if (m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT)
			{
				createSampler();
				changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			// Resolve the color attachment usage.
			else if (m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			{
				changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}

			// Resolve the storage usage.
			else if (m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT)
			{
				changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
			}

			// Resolve the depth stencil attachment usage.
			else if (m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				//changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);	// TODO
				m_CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
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
		 * Copy data from a stagging buffer.
		 *
		 * @param pBuffer The buffer pointer to copy data from.
		 */
		void fromBuffer(const Buffer* pBuffer)
		{
			VkBufferImageCopy vImageCopy = {};
			vImageCopy.imageExtent = m_Extent;
			vImageCopy.imageOffset = {};
			vImageCopy.imageSubresource.aspectMask = getImageAspectFlags();
			vImageCopy.imageSubresource.baseArrayLayer = 0;
			vImageCopy.imageSubresource.layerCount = m_Layers;
			vImageCopy.imageSubresource.mipLevel = 0;
			vImageCopy.bufferOffset = 0;
			vImageCopy.bufferRowLength = m_Extent.width;
			vImageCopy.bufferImageHeight = m_Extent.height;

			const auto oldlayout = m_CurrentLayout;
			const auto vCommandBuffer = getEngine()->beginCommandBufferRecording();

			// Change the layout to transfer source
			changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vCommandBuffer);

			// Copy the image.
			getEngine()->getDeviceTable().vkCmdCopyBufferToImage(vCommandBuffer, pBuffer->getBuffer(), m_vImage, m_CurrentLayout, 1, &vImageCopy);

			// Get it back to the old layout.
			if (oldlayout != VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && oldlayout != VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED)
				changeImageLayout(oldlayout, vCommandBuffer);

			// Execute the commands.
			getEngine()->executeRecordedCommands();
		}

		/**
		 * Copy the whole image to a buffer.
		 *
		 * @return The copied buffer.
		 */
		std::shared_ptr<Buffer> toBuffer()
		{
			const auto size = static_cast<uint64_t>(m_Extent.width) * m_Extent.height * m_Extent.depth * getPixelSize();
			auto pBuffer = Buffer::create(getEngine(), size, BufferType::Staging);

			VkBufferImageCopy vImageCopy = {};
			vImageCopy.imageExtent = m_Extent;
			vImageCopy.imageOffset = {};
			vImageCopy.imageSubresource.aspectMask = getImageAspectFlags();
			vImageCopy.imageSubresource.baseArrayLayer = 0;
			vImageCopy.imageSubresource.layerCount = m_Layers;
			vImageCopy.imageSubresource.mipLevel = 0;
			vImageCopy.bufferOffset = 0;
			vImageCopy.bufferRowLength = m_Extent.width;
			vImageCopy.bufferImageHeight = m_Extent.height;

			const auto oldlayout = m_CurrentLayout;
			const auto vCommandBuffer = getEngine()->beginCommandBufferRecording();

			// Change the layout to transfer source
			changeImageLayout(VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vCommandBuffer);

			// Copy the image.
			getEngine()->getDeviceTable().vkCmdCopyImageToBuffer(vCommandBuffer, m_vImage, m_CurrentLayout, pBuffer->getBuffer(), 1, &vImageCopy);

			// Get it back to the old layout.
			if (oldlayout != VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && oldlayout != VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED)
				changeImageLayout(oldlayout, vCommandBuffer);

			// Execute the commands.
			getEngine()->executeRecordedCommands();

			return pBuffer;
		}


		/**
		 * Change the image layout to another one.
		 */
		void changeImageLayout(VkImageLayout newLayout, VkCommandBuffer vCommandBuffer = VK_NULL_HANDLE)
		{
			// Create the memory barrier.
			VkImageMemoryBarrier vMemoryBarrier = {};
			vMemoryBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vMemoryBarrier.oldLayout = m_CurrentLayout;
			vMemoryBarrier.newLayout = newLayout;
			vMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vMemoryBarrier.image = m_vImage;

			if (m_CurrentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
				m_CurrentLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
				m_CurrentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
				newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
				newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
				newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			{
				vMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (hasStencilComponent())
					vMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
				vMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			vMemoryBarrier.subresourceRange.baseMipLevel = 0;
			vMemoryBarrier.subresourceRange.levelCount = 1;
			vMemoryBarrier.subresourceRange.layerCount = m_Layers;
			vMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			vMemoryBarrier.srcAccessMask = 0;
			vMemoryBarrier.dstAccessMask = 0;

			// Resolve the source access masks.
			switch (m_CurrentLayout)
			{
			case VK_IMAGE_LAYOUT_GENERAL:
			case VK_IMAGE_LAYOUT_UNDEFINED:
				vMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				//vMB.srcAccessMask = VK_ACCESS_;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				vMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;

			default:
				throw BackendError("Unsupported layout transition!");
			}

			// Resolve the destination access masks.
			switch (newLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
			case VK_IMAGE_LAYOUT_GENERAL:
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				vMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				vMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				vMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				vMemoryBarrier.dstAccessMask = vMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				vMemoryBarrier.srcAccessMask |= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				vMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;

			default:
				throw BackendError("Unsupported layout transition!");
			}

			// Resolve the pipeline stages.
			const VkPipelineStageFlags sourceStage = getPipelineStageFlags(vMemoryBarrier.srcAccessMask);
			const VkPipelineStageFlags destinationStage = getPipelineStageFlags(vMemoryBarrier.dstAccessMask);

			// Issue the commands. 
			// Here we begin the buffer recording if a command buffer was not given.
			if (vCommandBuffer == VK_NULL_HANDLE)
			{
				vCommandBuffer = getEngine()->beginCommandBufferRecording();
				getEngine()->getDeviceTable().vkCmdPipelineBarrier(vCommandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &vMemoryBarrier);
				getEngine()->executeRecordedCommands();
			}
			else
				getEngine()->getDeviceTable().vkCmdPipelineBarrier(vCommandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &vMemoryBarrier);

			m_CurrentLayout = newLayout;
		}

		/**
		 * Terminate the image.
		 */
		void terminate() override
		{
			// Terminate the sampler if created.
			if (m_vSampler != VK_NULL_HANDLE)
				getEngine()->getDeviceTable().vkDestroySampler(getEngine()->getLogicalDevice(), m_vSampler, nullptr);

			getEngine()->getDeviceTable().vkDestroyImageView(getEngine()->getLogicalDevice(), m_vImageView, nullptr);
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
		 * @param usageFlags The image usage flags. Default is sampled | transfer source | transfer destination.
		 * @return The image pointer.
		 */
		static std::shared_ptr<Image> create(const std::shared_ptr<Engine>& pEngine, const VkExtent3D extent, const VkFormat format, const ImageType type, const uint32_t layers = 1,
			const VkImageUsageFlags usageFlags = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			return std::make_shared<Image>(pEngine, extent, format, type, layers, usageFlags);
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

		/**
		 * Get the image layout.
		 *
		 * @return The image layout.
		 */
		VkImageLayout getImageLayout() const { return m_CurrentLayout; }

		/**
		 * Get the byte depth of the current format.
		 *
		 * @return The pixel size in bytes.
		 */
		uint8_t getPixelSize() const
		{
			switch (m_Format)
			{
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SNORM:
			case VK_FORMAT_R8_USCALED:
			case VK_FORMAT_R8_SSCALED:
			case VK_FORMAT_R8_UINT:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_SRGB:
			case VK_FORMAT_S8_UINT:
				return 1;

			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_USCALED:
			case VK_FORMAT_R8G8_SSCALED:
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
			case VK_FORMAT_R16_UNORM:
			case VK_FORMAT_R16_SNORM:
			case VK_FORMAT_R16_USCALED:
			case VK_FORMAT_R16_SSCALED:
			case VK_FORMAT_R16_UINT:
			case VK_FORMAT_R16_SINT:
			case VK_FORMAT_R16_SFLOAT:
			case VK_FORMAT_D16_UNORM:
				return 2;

			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SNORM:
			case VK_FORMAT_R8G8B8_USCALED:
			case VK_FORMAT_R8G8B8_SSCALED:
			case VK_FORMAT_R8G8B8_UINT:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_SRGB:
			case VK_FORMAT_B8G8R8_UNORM:
			case VK_FORMAT_B8G8R8_SNORM:
			case VK_FORMAT_B8G8R8_USCALED:
			case VK_FORMAT_B8G8R8_SSCALED:
			case VK_FORMAT_B8G8R8_UINT:
			case VK_FORMAT_B8G8R8_SINT:
			case VK_FORMAT_B8G8R8_SRGB:
			case VK_FORMAT_D16_UNORM_S8_UINT:
				return 3;

			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SNORM:
			case VK_FORMAT_R8G8B8A8_USCALED:
			case VK_FORMAT_R8G8B8A8_SSCALED:
			case VK_FORMAT_R8G8B8A8_UINT:
			case VK_FORMAT_R8G8B8A8_SINT:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SNORM:
			case VK_FORMAT_B8G8R8A8_USCALED:
			case VK_FORMAT_B8G8R8A8_SSCALED:
			case VK_FORMAT_B8G8R8A8_UINT:
			case VK_FORMAT_B8G8R8A8_SINT:
			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			case VK_FORMAT_R16G16_UNORM:
			case VK_FORMAT_R16G16_SNORM:
			case VK_FORMAT_R16G16_USCALED:
			case VK_FORMAT_R16G16_SSCALED:
			case VK_FORMAT_R16G16_UINT:
			case VK_FORMAT_R16G16_SINT:
			case VK_FORMAT_R16G16_SFLOAT:
			case VK_FORMAT_R32_UINT:
			case VK_FORMAT_R32_SINT:
			case VK_FORMAT_R32_SFLOAT:
			case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
				return 4;

			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return 5;

			case VK_FORMAT_R16G16B16_UNORM:
			case VK_FORMAT_R16G16B16_SNORM:
			case VK_FORMAT_R16G16B16_USCALED:
			case VK_FORMAT_R16G16B16_SSCALED:
			case VK_FORMAT_R16G16B16_UINT:
			case VK_FORMAT_R16G16B16_SINT:
			case VK_FORMAT_R16G16B16_SFLOAT:
				return 6;

			case VK_FORMAT_R16G16B16A16_UNORM:
			case VK_FORMAT_R16G16B16A16_SNORM:
			case VK_FORMAT_R16G16B16A16_USCALED:
			case VK_FORMAT_R16G16B16A16_SSCALED:
			case VK_FORMAT_R16G16B16A16_UINT:
			case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32_UINT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_SFLOAT:
			case VK_FORMAT_R64_UINT:
			case VK_FORMAT_R64_SINT:
			case VK_FORMAT_R64_SFLOAT:
				return 8;

			case VK_FORMAT_R32G32B32_UINT:
			case VK_FORMAT_R32G32B32_SINT:
			case VK_FORMAT_R32G32B32_SFLOAT:
				return 12;

			case VK_FORMAT_R32G32B32A32_UINT:
			case VK_FORMAT_R32G32B32A32_SINT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
			case VK_FORMAT_R64G64_UINT:
			case VK_FORMAT_R64G64_SINT:
			case VK_FORMAT_R64G64_SFLOAT:
				return 16;

			case VK_FORMAT_R64G64B64_UINT:
			case VK_FORMAT_R64G64B64_SINT:
			case VK_FORMAT_R64G64B64_SFLOAT:
				return 24;

			case VK_FORMAT_R64G64B64A64_UINT:
			case VK_FORMAT_R64G64B64A64_SINT:
			case VK_FORMAT_R64G64B64A64_SFLOAT:
				return 32;

			case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
			case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
			case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			case VK_FORMAT_BC2_UNORM_BLOCK:
			case VK_FORMAT_BC2_SRGB_BLOCK:
			case VK_FORMAT_BC3_UNORM_BLOCK:
			case VK_FORMAT_BC3_SRGB_BLOCK:
			case VK_FORMAT_BC4_UNORM_BLOCK:
			case VK_FORMAT_BC4_SNORM_BLOCK:
			case VK_FORMAT_BC5_UNORM_BLOCK:
			case VK_FORMAT_BC5_SNORM_BLOCK:
			case VK_FORMAT_BC6H_UFLOAT_BLOCK:
			case VK_FORMAT_BC6H_SFLOAT_BLOCK:
			case VK_FORMAT_BC7_UNORM_BLOCK:
			case VK_FORMAT_BC7_SRGB_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
			case VK_FORMAT_EAC_R11_UNORM_BLOCK:
			case VK_FORMAT_EAC_R11_SNORM_BLOCK:
			case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
			case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
			case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
			case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
			case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
			case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
			case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
			case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
			case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
			case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
			case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
			case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
			case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
			case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
			case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
			case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
			case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
			case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
			case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
			case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
			case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
			case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
			case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
			case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
			case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
			case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
			case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
			case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
			case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
			case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
			case VK_FORMAT_G8B8G8R8_422_UNORM:
			case VK_FORMAT_B8G8R8G8_422_UNORM:
			case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
			case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
			case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
			case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
			case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
			case VK_FORMAT_R10X6_UNORM_PACK16:
			case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
			case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
			case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
			case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
			case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
			case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
			case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
			case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
			case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_R12X4_UNORM_PACK16:
			case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
			case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
			case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
			case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
			case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_G16B16G16R16_422_UNORM:
			case VK_FORMAT_B16G16R16G16_422_UNORM:
			case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
			case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
			case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
			case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
			case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
			case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
			case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
			case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
			case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
			case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
			case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
			case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
			case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
			case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
			case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
			case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
			case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
			case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
			case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
			case VK_FORMAT_MAX_ENUM:
				break;
			}

			return 0;
		}

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
			vCreateInfo.initialLayout = m_CurrentLayout;
			vCreateInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
			vCreateInfo.queueFamilyIndexCount = 0;
			vCreateInfo.pQueueFamilyIndices = nullptr;
			vCreateInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
			vCreateInfo.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
			vCreateInfo.usage = m_UsageFlags;
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
			vCreateInfo.subresourceRange.aspectMask = getImageAspectFlags();
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
			VkSamplerAddressMode addressMode = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;

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

		/**
		 * Get the pipeline stage masks.
		 *
		 * @param flags The access flags.
		 * @retrurn The pipeline stage flags.
		 */
		VkPipelineStageFlags getPipelineStageFlags(VkAccessFlags flags) const
		{
			switch (flags)
			{
			case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:						return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			case VK_ACCESS_INDEX_READ_BIT:									return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:						return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			case VK_ACCESS_UNIFORM_READ_BIT:								return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			case VK_ACCESS_SHADER_READ_BIT:									return VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			case VK_ACCESS_SHADER_WRITE_BIT:								return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:						return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:						return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:						return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:				return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:				return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case VK_ACCESS_TRANSFER_READ_BIT:								return VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			case VK_ACCESS_TRANSFER_WRITE_BIT:								return VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			case VK_ACCESS_HOST_READ_BIT:									return VK_PIPELINE_STAGE_HOST_BIT;
			case VK_ACCESS_HOST_WRITE_BIT:									return VK_PIPELINE_STAGE_HOST_BIT;
			case VK_ACCESS_MEMORY_READ_BIT:									return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			case VK_ACCESS_MEMORY_WRITE_BIT:								return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			case VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT:		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV:					return VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;
			case VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV:					return VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;
			case VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT:				return VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
			case VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR:	return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
				//case VK_ACCESS_2_INVOCATION_MASK_READ_BIT_HUAWEI:				return VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI;
			case VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:				return VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
			case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:		return VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
			case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT:			return VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			case VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR:				return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			case VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR:			return VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			case VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT:				return VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
			}

			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		}

		/**
		 * Check if our format has a stencil component.
		 *
		 * @return Boolean stating if we do or not.
		 */
		bool hasStencilComponent() const
		{
			return m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_Format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		/**
		 * Get the required aspect flags.
		 *
		 * @return The image aspect flags.
		 */
		VkImageAspectFlags getImageAspectFlags() const
		{
			if (m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;

			else if (m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ||
				m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT ||
				m_UsageFlags & VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT)
				return VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;

			return 0;
		}

	private:
		VkExtent3D m_Extent;

		VkImage m_vImage = VK_NULL_HANDLE;
		VkImageView m_vImageView = VK_NULL_HANDLE;
		VkSampler m_vSampler = VK_NULL_HANDLE;

		VmaAllocation m_Allocation = nullptr;

		VkImageLayout m_CurrentLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		VkFormat m_Format = VkFormat::VK_FORMAT_UNDEFINED;
		ImageType m_Type = ImageType::TwoDimension;
		uint32_t m_Layers = 0;
		VkImageUsageFlags m_UsageFlags = 0;
	};
}