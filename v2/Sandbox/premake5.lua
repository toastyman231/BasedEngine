include "utils.lua"

ENGINE_DIR, SUCCESS, VAL = Utils.GetEngineInstallDir()
if not SUCCESS then print("Error getting engine directory: " .. VAL) end

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    links
    {
        "based",
        "jolt",
        "tlsf",
        "assimp",
        "freetype",
        "ktx-software",
        "rmlui",
        "spdlog",
        "tracy",
        "yaml-cpp"
    }

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "../based/include",
        ENGINE_DIR .. "/external/Public",
        ENGINE_DIR .. "/external/Private/spdlog/include",
        --[[
        "%{externals.rmlui}",
        "%{externals.tracy}",
        "%{externals.yaml_cpp}",
        "%{externals.jolt}" ]]
    }

    libdirs
    {
        --[[ "%{libraries.sdl2}",
        "%{libraries.freetype}",
        "%{libraries.assimp}",
        "%{libraries.rmlui}",
        "%{libraries.ktx_software}" ]]
    }

    fatalwarnings "All"

    postbuildcommands
    {
        "python3 " .. ENGINE_DIR .. "/tools/basedbuildtool.py -i " .. path.getabsolute("../%{prj.name}") .. " -c %{cfg.buildcfg} -q 0.05 --ci" 
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        --files { "resources.rc", "Assets/**.ico" }
        --vpaths { ['Assets/*'] = { '*.rc', '**.ico' } }
        debugdir("bin/%{cfg.buildcfg}/%{prj.name}")

        defines
        {
            "BASED_PLATFORM_WINDOWS",
            "JPH_DEBUG_RENDERER"
        }

        links
        {
            "Winmm",     -- Fixes timeBeginPeriod / timeEndPeriod
            "Setupapi",  -- Fixes SetupDiGetClassDevsA / SetupDiGetDeviceInstanceIdA
            "Version",   -- Required for SDL Windows version checks
            "Imm32"      -- Required for SDL Input Method Editor (IME) support
        }

    filter {"system:windows", "configurations:Release*"}
        kind "WindowedApp"
    filter {"system:macosx", "configurations:Release*"}
        kind "WindowedApp"

    filter {"system:linux", "configurations:*"}
        defines
        {
            "BASED_PLATFORM_LINUX"
        }
    
    filter "configurations:Debug*"
        defines "BASED_CONFIG_DEBUG"
        runtime "Debug"
        editandcontinue "off"
        symbols "on"
        optimize "debug"
        targetsuffix "_d"

    filter "configurations:Development*"
        defines 
        {
            "BASED_CONFIG_DEBUG",
            "BASED_CONFIG_DEVELOPMENT"
        }
        runtime "Release"
        editandcontinue "off"
        optimize "on"
        symbols "on"
        targetsuffix "_dev"

    filter "configurations:Release*"
        defines "BASED_CONFIG_RELEASE"
        fatalwarnings "All"
        runtime "Release"
        symbols "off"
        optimize "on"

    filter "configurations:*Editor"
        defines "BASED_CONFIG_EDITOR"

    filter { "system:windows", "configurations:Debug*" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build/Debug" }
        links { "SDL3-static" }

    filter { "system:windows", "configurations:Development*" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build/RelWithDebInfo" }
        links { "SDL3-static" }

    filter { "system:windows", "configurations:Release*" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build/Release" }
        links { "SDL3-static" }

    filter { "system:linux", "configurations:Debug*" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build/Debug" }
        links { "SDL3" }

    filter { "system:linux", "configurations:Development*" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build/RelWithDebInfo" }
        links { "SDL3" }

    filter { "system:linux", "configurations:Release*" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build/Release" }
        links { "SDL3" }