workspace "GraphcisCore"
	architecture "x64"

	configurations {
		"Debug",
		"Release"
	}

	filter "configurations:Debug"
		defines { "GraphicsCore_DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "GraphicsCore_RELEASE" }
		optimize "On"
		runtime "Release"

	filter "system:windows"
		defines { "GraphicsCore_PLATFORM_WINDOWS" }

	filter "system:linux"
		defines { "GraphicsCore_PLATFORM_LINUX" }

	filter ""

	-- Include the projects.
	include "Include/GraphicsCore.lua"
	include "Test/Test.lua"