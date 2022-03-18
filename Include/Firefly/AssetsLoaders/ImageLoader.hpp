#pragma once

#include <filesystem>
#include "Firefly/Image.hpp"

namespace Firefly
{
	/**
	 * Image data format enum.
	 */
	enum class ImageDataFormat : uint8_t
	{
		Bitmap,
		PNG,
		JPEG
	};

	/**
	 * Load an image from a given directory.
	 *
	 * @param pEngine The engine pointer to which the image is bound to.
	 * @param path The file path.
	 * @return The created image.
	 */
	std::shared_ptr<Image> LoadImageFromFile(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& path);

	/**
	 * Load an image from memory.
	 *
	 * @param pEngine The engine pointer.
	 * @param pImageData The image data to load.
	 * @param size The image data size.
	 * @param format The image data format.
	 * @return The created image.
	 */
	std::shared_ptr<Image> LoadImageFromMemory(const std::shared_ptr<Engine>& pEngine, const unsigned char* pImageData, const uint64_t size, const ImageDataFormat format);
}