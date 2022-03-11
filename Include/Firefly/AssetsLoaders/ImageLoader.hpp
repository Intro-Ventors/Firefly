#pragma once

#include <stb/stb_image.h>
#include <filesystem>
#include "Firefly/Image.hpp"

namespace Firefly
{
	/**
	 * Load an image from a given directory.
	 * 
	 * @param pEngine The engine pointer to which the image is bound to.
	 * @param path The file path.
	 * @return The created image.
	 */
	std::shared_ptr<Image> LoadImage(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& path);
}