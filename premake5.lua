workspace "RCHAC"
	architecture "x64"

	configurations {
		"Debug",
		"Release"
	}

	-- Setup the dependencies.
	IncludeDirectories = {}
	LibraryDirectories = {}
	Binaries = {}

	VulkanSDKPath = path.translate(os.getenv("VULKAN_SDK"))

	Binaries["Vulkan"] = "vulkan-1"

	filter "configurations:Debug"
		defines { "RCHAC_DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "RCHAC_RELEASE" }
		optimize "On"
		runtime "Release"

	filter "system:windows"
		defines { "RCHAC_PLATFORM_WINDOWS" }
		LibraryDirectories["Vulkan"] = VulkanSDKPath .. "/Lib/"
		IncludeDirectories["Vulkan"] = VulkanSDKPath .. "/Include/"

	filter "system:linux"
		defines { "RCHAC_PLATFORM_LINUX" }
		LibraryDirectories["Vulkan"] = VulkanSDKPath .. "/lib/"
		IncludeDirectories["Vulkan"] = VulkanSDKPath .. "/include/"

	filter ""

	-- Include the projects.
	include "Include/RCHAC.lua"
	include "Test/Test.lua"