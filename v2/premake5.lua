include "helpers"

workspace "based"
    startproject "BasedEditor"
    architecture "x64"
    staticruntime "off"

    newoption
    {
        trigger = "targetplatform",
        value = "PLATFORM",
        description = "Select target platform"
    }

    newoption
    {
        trigger = "outputhtml",
        description = "When set will output an html file that loads the generated wasm."
    }

    configurations
    {
        "DebugGame",
        "DebugEditor",
        "DevelopmentGame",
        "DevelopmentEditor",
        "ReleaseGame",
        "ReleaseEditor"
    }

    local targetPlatform = _OPTIONS["targetplatform"]

    if targetPlatform == "Web" then
        platforms { "Web" }
    else
        platforms
        {
            "Win64",
            -- "Win64-DX12" -- Coming soon! (Maybe)
            "Win64-MSVC",
            -- "Win64-DX12-MSVC" -- Coming soon! (Maybe)
            "Linux",
            "Web"
        }
    end

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
    }

    defaultplatform "Win64"

    multiprocessorcompile "On"

    local engine_path = 

    filter "platforms:Win64"
        system "windows"
        toolset "clang"
        defines { "_CXX20_DEPRECATE_OLD_SHARED_PTR_ATOMIC_SUPPORT" }
        forceincludes { _MAIN_SCRIPT_DIR  .. "/based/include/based/core/NewDelete.h" }
    filter {}

    filter "platforms:Win64-MSVC"
        system "windows"
        toolset "msc"
        buildoptions 
        { 
            "/utf-8", 
            "/FI\"" .. path.translate(_MAIN_SCRIPT_DIR .. "/based/include/based/core/NewDelete.h") .. "\"" 
        }

    filter {}

    filter "platforms:Web"
        system "emscripten"
        toolset "emcc"
    filter {}

    filter "options:outputhtml"
        targetextension ".html"
    filter {}

    tdir = "bin/%{cfg.system}/%{cfg.buildcfg}/%{prj.name}"
    odir = "bin-obj/%{cfg.system}/%{cfg.buildcfg}/%{prj.name}"

    filter "platforms:Linux"
        system "linux"
        if os.host() == "windows" then
            toolset "clang-wsl2"
        else
            toolset "clang"
        end
    filter {}

    filter "system:windows"
        defines { "BASED_PLATFORM_WINDOWS" }
    filter {}

    filter "system:linux"
        defines { "BASED_PLATFORM_LINUX" }
    filter {}

    filter "system:macosx"
        defines { "BASED_PLATFORM_MAC" }
    filter {}

    filter "system:emscripten"
        defines { "BASED_PLATFORM_WEB" }
    filter {}

    filter "configurations:Debug*"
        defines { "BASED_CONFIG_DEBUG" }
    filter {}

    filter "configurations:Development*"
        defines
        { 
            "BASED_CONFIG_DEBUG", 
            "BASED_CONFIG_DEVELOPMENT"
        }
    filter {}

    filter "configurations:Release*"
        defines { "BASED_CONFIG_RELEASE" }
    filter {}

    filter "configurations:*Editor"
        defines "BASED_CONFIG_EDITOR"
    filter {}
    
    group "Core"
        include "based"
    group ""

    group "Dependencies"
        include "external/Private/assimp"
        include "external/Private/freetype"
        include "external/Private/jolt"
        include "external/Private/ktx-software"
        include "external/Private/rmlui"
        include "external/Private/spdlog"
        include "external/Private/tlsf"
        include "external/Private/tracy"
        include "external/Private/yaml-cpp"

        filter { "platforms:Linux or Win64* or Web" }
            include "external/Private/sdl3"
        filter {}

        project "Public"
            buildaction "None"
            kind "None"
            location "../Intermediate"
            files 
            {
                "external/Public/**.*"
            }
    group ""

    group "Tools"
        --include "BasedEditor"
    group ""

    group "Misc"
        include "Sandbox"
    group ""
