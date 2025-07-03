project "MinHook"
	kind "StaticLib"
	language "C++"
    staticruntime "off"
	systemversion "latest"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
		"./**.c",
		"./**.h",
	}

	-- defines { "WIN32", "_LIB", "_MT", "STRICT" }
	defines { "WIN32", "STRICT" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		defines { "_DEBUG" }
		
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		defines { "NDEBUG" }
		
	filter "configurations:Test"
		runtime "Debug"
		symbols "on"
		defines { "_DEBUG" }
