project "RCHAC"
	language "C++"
	kind "StaticLib"
	systemversion "latest"
	cppdialect "C++20"
	staticruntime "on"

	flags { "MultiProcessorCompile" }

	targetdir "%{wks.location}/Builds/%{cfg.longname}"
	objdir "%{wks.location}/Builds/Intermediate/%{cfg.longname}"

	files {
		"**.txt",
		"**.cpp",
		"**.hpp",
		"**.lua",
		"**.txt",
		"**.md",
	}

	includedirs {
		"%{wks.location}/Include/",
		"%{IncludeDirectories.Vulkan}",
	}
	
	libdirs {
		"%{LibraryDirectories.Vulkan}",
	}
	
	links { 
		"%{Binaries.Vulkan}",
	}

	filter { "toolset:msc", "configurations:Debug" }
		buildoptions "/MTd"

	filter { "toolset:msc", "configurations:Release" }
		buildoptions "/MT"

	filter ""