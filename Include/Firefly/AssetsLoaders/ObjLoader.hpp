#pragma once

#include <filesystem>
#include "Types.hpp"

namespace Firefly
{
	/**
	 * Load an obj file.
	 *
	 * @param pEngine The engine to which the model is bound to.
	 * @param path The object file path.
	 * @return The loaded model.
	 * @reference https://vulkan-tutorial.com/code/30_multisampling.cpp
	 */
	ObjModel LoadObjModel(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& path);
}