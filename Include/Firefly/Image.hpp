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
			const VkImageUsageFlags usageFlags = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT);

		/**
		 * Destructor.
		 */
		~Image() override;

		/**
		 * Copy data from a stagging buffer.
		 *
		 * @param pBuffer The buffer pointer to copy data from.
		 */
		void fromBuffer(const Buffer* pBuffer);

		/**
		 * Copy the whole image to a buffer.
		 *
		 * @return The copied buffer.
		 */
		std::shared_ptr<Buffer> toBuffer();

		/**
		 * Change the image layout to another one.
		 *
		 * @param newLayout The new layout to set.
		 * @param vCommandBuffer The command buffer used to send the commands to the GPU. Default is NULL.
		 */
		void changeImageLayout(const VkImageLayout newLayout, const VkCommandBuffer vCommandBuffer = VK_NULL_HANDLE);

		/**
		 * Terminate the image.
		 */
		void terminate() override;

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
			const VkImageUsageFlags usageFlags = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT);

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
		uint8_t getPixelSize() const;

	private:
		/**
		 * Create the image.
		 */
		void createImage();

		/**
		 * Create the image view.
		 */
		void createImageView();

		/**
		 * Create image sampler.
		 */
		void createSampler();

		/**
		 * Get the pipeline stage masks.
		 *
		 * @param flags The access flags.
		 * @retrurn The pipeline stage flags.
		 */
		VkPipelineStageFlags getPipelineStageFlags(VkAccessFlags flags) const;

		/**
		 * Check if our format has a stencil component.
		 *
		 * @return Boolean stating if we do or not.
		 */
		bool hasStencilComponent() const { return m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_Format == VK_FORMAT_D24_UNORM_S8_UINT; }

		/**
		 * Get the required aspect flags.
		 *
		 * @return The image aspect flags.
		 */
		VkImageAspectFlags getImageAspectFlags() const;

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