workspace "based"
    startproject "BasedEditor"
    architecture "x64"

    configurations
    {
        "DebugGame",
        "DebugEditor",
        "DevelopmentGame",
        "DevelopmentEditor",
        "ReleaseGame",
        "ReleaseEditor"
    }

    platforms
    {
        "Win64",
        -- "Win64-DX12" -- Coming soon! (Maybe)
        "Win64-MSVC",
        -- "Win64-DX12-MSVC" -- Coming soon! (Maybe)
        "Linux",
        "Web"
    }

    defaultplatform "Win64"

    multiprocessorcompile "On"

    filter "platforms:Win64"
        system "windows"
        toolset "clang"

    filter "platforms:Win64-MSVC"
        system "windows"
        toolset "msc"

    filter "platforms:Linux"
        system "linux"
        if os.host() == "windows" then
            toolset "clang-wsl2"
        else
            toolset "clang"
        end
    
    group "Core"
        include "based"
    group ""

    group "Dependencies"
        include "external/Private/assimp"
        include "external/Private/freetype"
        include "external/Private/jolt"
        include "external/Private/ktx-software"
        include "external/Private/rmlui"
        include "external/Private/sdl3"
        include "external/Private/spdlog"
        include "external/Private/tracy"
        include "external/Private/yaml-cpp"
    group ""

    group "Tools"
        --include "BasedEditor"
    group ""

    group "Misc"
        include "Sandbox"
    group ""
