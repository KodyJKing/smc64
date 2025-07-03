project "smc64-launcher"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    staticruntime "off"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../obj/" .. outputdir .. "/%{prj.name}")

    files {
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs {
        "src"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
  
    filter "configurations:Release"
        runtime "Release"
        optimize "On"
