project "Firefly"
	language "C++"
	kind "StaticLib"
	systemversion "latest"
	cppdialect "C++17"
	staticruntime "on"

	flags { "MultiProcessorCompile" }

	targetdir "%{wks.location}/Builds/%{cfg.longname}"
	objdir "%{wks.location}/Builds/Intermediate/%{cfg.longname}"

	files {
		"Firefly/**.txt",
		"Firefly/**.cpp",
		"Firefly/**.hpp",
		"Firefly/**.lua",
		"Firefly/**.txt",
		"Firefly/**.md",
	}

	includedirs {
		"%{wks.location}/Include/",
	}
	
	libdirs {
	}
	
	links { 
	}

	filter { "toolset:msc", "configurations:Debug" }
		buildoptions "/MTd"

	filter { "toolset:msc", "configurations:Release" }
		buildoptions "/MT"

	filter ""