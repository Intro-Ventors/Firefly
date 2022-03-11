#include "Firefly/AssetsLoaders/ImageLoader.hpp"

namespace Firefly
{
	std::shared_ptr<Image> LoadImage(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& path)
	{
		// Load the pixel data.
		int width = 0, height = 0, channels = 0;
		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

		// Check if we were able to load the image.
		if (!pixels)
			throw BackendError("Could not load the asset image!");

		// Create the copy buffer and copy the content to it.
		const uint64_t imageSize = static_cast<uint64_t>(width) * height * channels;
		auto pCopyBuffer = Firefly::Buffer::create(pEngine, imageSize, Firefly::BufferType::Staging);

		std::copy(pixels, pixels + imageSize, reinterpret_cast<stbi_uc*>(pCopyBuffer->mapMemory()));
		pCopyBuffer->unmapMemory();

		// Create the image and copy the buffer to it.
		auto pTexture = Firefly::Image::create(pEngine, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 }, VkFormat::VK_FORMAT_R8G8B8A8_SRGB, Firefly::ImageType::TwoDimension);
		pTexture->fromBuffer(pCopyBuffer.get());

		std::free(pixels);
		return pTexture;
	}
}