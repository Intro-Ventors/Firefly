workspace "Firefly"
	architecture "x64"

	configurations {
		"Debug",
		"Release"
	}

	-- Libraries
	IncludeDir = {}

	-- Binary includes directories
	IncludeLib = {}
	IncludeLib["GLFW"] = "%{wks.location}/ThirdParty/GLFW/build/src/Release"

	-- Binaries
	Binary = {}
	Binary["GLFW"] = "glfw3dll"


	filter "configurations:Debug"
		defines { "FIREFLY_DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "FIREFLY_RELEASE" }
		optimize "On"
		runtime "Release"

	filter "system:windows"
		defines { "FIREFLY_PLATFORM_WINDOWS" }

	filter "system:linux"
		defines { "FIREFLY_PLATFORM_LINUX" }

	filter ""

	-- Include the projects.
	include "Include/Firefly.lua"
	include "Test/Test.lua"