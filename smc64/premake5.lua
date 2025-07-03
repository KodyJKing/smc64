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
    }

    includedirs {
        "src",
        "../vendor/minhook/include",
        "../vendor/imgui/backends",
        "../vendor/imgui",
        "../vendor/zydis",
    }

    pchheader "pch.h"
    pchsource "pch.cpp"
    forceincludes "pch.h"

    links { 
        "MinHook",
        "Zydis",
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

-- local function updateBuildNum()

--     local filePath = "./src/buildnum.h"

--     local file = io.open(filePath, "r")
--     if file == nil then
--         print("Error: Could not open file " .. filePath)
--         os.exit(1)
--     end
--     local text = file:read("a")
--     file:close()
--     local numText = string.gsub(text, "#define BUILDNUM ", "")
--     local buildNum = tonumber(numText)
--     if buildNum == nil then
--         print("Error: Could not parse build number from " .. numText)
--         os.exit(1)
--     end
--     buildNum = buildNum + 1

--     file = io.open(filePath, "w")
--     if file == nil then
--         print("Error: Could not open file " .. filePath)
--         os.exit(1)
--     end
--     file:write("#define BUILDNUM " .. buildNum .. "\n")
--     file:close()

-- end
-- updateBuildNum()
