project "smc64"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    staticruntime "off"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../obj/" .. outputdir .. "/%{prj.name}")

    files {
        "pch.h",
        "pch.cpp",
        "src/**.hpp",
        "src/**.cpp",
        "../vendor/imgui/*.h", 
        "../vendor/imgui/*.cpp",
        "../vendor/imgui/backends/imgui_impl_win32.*",
        "../vendor/imgui/backends/imgui_impl_dx11.*",
        "../vendor/libsm64/include/*.h"
    }

    includedirs {
        "src",
        "../vendor/minhook/include",
        "../vendor/imgui/backends",
        "../vendor/imgui",
        "../vendor/zydis",
        "../vendor/libsm64/include",
    }

    pchheader "pch.h"
    pchsource "pch.cpp"
    forceincludes "pch.h"

    links { 
        "MinHook",
        "Zydis",
        "../vendor/libsm64/sm64",
    }

    filter "system:windows"
        systemversion "latest"
        files { 'versioninfo.rc' }
        vpaths { ['Resources/*'] = { '*.rc' } }

    filter "configurations:Debug"
        runtime "Debug"

        --  Turn symbols off when you're using Cheat Engine as part of your workflow. 
        --  Cheat Engine will keep .pdb files open and prevent the linker from writing to them.
        -- symbols "On"
        symbols "Off"
  
    filter "configurations:Release"
        runtime "Release"
        optimize "On"
