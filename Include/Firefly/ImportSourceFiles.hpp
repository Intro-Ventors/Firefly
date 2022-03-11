#pragma once

/**
 * This file imports all the source files from this library.
 * Make sure that this file is included in ONE SOURCE FILE to compile the whole library. No additional dependencies needed.
 */

#include "Source/AssetLoaders/ImageLoader.cpp"
#include "Source/AssetLoaders/ObjLoader.cpp"
#include "Source/AssetLoaders/Types.cpp"

#include "Source/Decoder/Decoder.cpp"
#include "Source/Encoder/Encoder.cpp"

#include "Source/Graphics/GraphicsEngine.cpp"
#include "Source/Graphics/GraphicsPipeline.cpp"
#include "Source/Graphics/Package.cpp"
#include "Source/Graphics/RenderTarget.cpp"

#include "Source/Maths/Camera.cpp"
#include "Source/Maths/CameraMatrix.cpp"
#include "Source/Maths/MonoCamera.cpp"
#include "Source/Maths/StereoCamera.cpp"

#include "Source/Buffer.cpp"
#include "Source/CommandBuffer.cpp"
#include "Source/Engine.cpp"
#include "Source/EngineBoundObject.cpp"
#include "Source/Image.cpp"
#include "Source/Instance.cpp"
#include "Source/Queue.cpp"
#include "Source/Shader.cpp"
#include "Source/Utility.cpp"

#include <SPIRV-Reflect/spirv_reflect.c>

#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#define VMA_IMPLEMENTATION
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>