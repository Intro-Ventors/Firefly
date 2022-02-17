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
	IncludeDirectories["Vulkan"] = "%{wks.location}/ThirdParty/Vulkan-Headers/include"

	if os.host() == "windows" then
		LibraryDirectories["Vulkan"] = VulkanSDKPath .. "/Lib/"
	else
		LibraryDirectories["Vulkan"] = VulkanSDKPath .. "/lib/"
	end

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

	filter "system:linux"
		defines { "RCHAC_PLATFORM_LINUX" }

	filter ""

	-- Include the projects.
	include "Include/RCHAC.lua"
	include "Test/Test.lua"