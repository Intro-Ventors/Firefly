project "Test"
	kind "ConsoleApp"
	cppdialect "C++20"
	language "C++"
	systemversion "latest"
	staticruntime "on"

	flags { "MultiProcessorCompile" }

	targetdir "%{wks.location}/Builds/%{cfg.longname}"
	objdir "%{wks.location}/Builds/Intermediate/%{cfg.longname}"

	files {
		"**.txt",
		"**.cpp",
		"**.hpp",
		"**.h",
		"**.lua",
	}

	includedirs {
		"%{wks.location}/Include",
		"%{wks.location}/Tests",
		"%{IncludeDirectories.Vulkan}",
	}

	libdirs {
	}

	links {
		"RCHAC",
	}

	filter { "toolset:msc", "configurations:Debug" }
	    buildoptions "/MTd"

	filter { "toolset:msc", "configurations:Release" }
	    buildoptions "/MT"

	filter ""