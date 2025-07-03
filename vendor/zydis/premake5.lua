project "Zydis"
	kind "StaticLib"
	language "C++"
    staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
		"./**.c",
		"./**.h",
	}

	-- Now this is defined in the top level premake file to ensure consistency in consumers.
	-- defines { "ZYDIS_STATIC_BUILD" }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++20"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		symbols "off"
