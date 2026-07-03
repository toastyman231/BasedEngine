project "sdl3"
    kind "StaticLib"
    language "C"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    -- Include all source files for IDE navigation and code completion
    -- The actual build is handled by CMake via the prebuild command
    files
    {
        "include/**.h",
    }

    includedirs
    {
        "include",
        "src",
        "src/video/khronos",
        "build/include-config-debug/build_config",
        "build/include-config-release/build_config",
        "build/include-config-relwithdebinfo/build_config",
        "build/include-revision",
    }

    defines
    {
        "SDL_STATIC_LIB",
        "USING_GENERATED_CONFIG_H",
        "SDL_BUILD_MAJOR_VERSION=3",
        "SDL_BUILD_MINOR_VERSION=4",
        "SDL_BUILD_MICRO_VERSION=12",
    }

    -- Disable premake's actual compilation of these files —
    -- CMake will produce the real lib via the prebuild command
    buildaction "None"

    enablepch "off"

    filter "system:windows"
        systemversion "latest"
        defines
        {
            "_CRT_SECURE_NO_DEPRECATE",
            "_CRT_NONSTDC_NO_DEPRECATE",
            "_CRT_SECURE_NO_WARNINGS",
            "WIN32_LEAN_AND_MEAN",
        }

    filter "system:linux"
        pic "On"

    filter "system:macosx"
        pic "On"

    -- Helper function to map Premake's _ACTION to CMake's -G flag
    local function get_cmake_generator()
        local action = _ACTION or ""
        
        if action:find("vs") then
            -- Matches vs2022, vs2019, etc.
            -- We escape the quotes with \" so the shell handles the spacing correctly
            if action == "vs2022" then return 'Visual Studio 17 2022' end
            if action == "vs2019" then return 'Visual Studio 16 2019' end
            return 'Visual Studio 17 2022' -- fallback default for VS
        elseif action:find("gmake") then
            -- Matches gmake, gmake2, etc. (No spaces, no quotes strictly required)
            return 'Unix Makefiles'
        elseif action == "xcode" then
            return 'Xcode'
        else
            return 'Unix Makefiles'
        end
    end

    local cmake_gen = get_cmake_generator()

    -- Debug configurations
    filter { "configurations:Debug*", "system:windows" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=Debug -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config Debug",
        }

    filter { "configurations:Debug*", "system:linux" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=Debug -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config Debug",
        }

    filter { "configurations:Debug*", "system:macosx" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=Debug -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config Debug",
        }

    -- Development configurations
    filter { "configurations:Development*", "system:windows" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config RelWithDebInfo",
        }

    filter { "configurations:Development*", "system:linux" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config RelWithDebInfo",
        }

    filter { "configurations:Development*", "system:macosx" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config RelWithDebInfo",
        }

    -- Release configurations
    filter { "configurations:Release*", "system:windows" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config Release",
        }

    filter { "configurations:Release*", "system:linux" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config Release",
        }

    filter { "configurations:Release*", "system:macosx" }
        prebuildcommands
        {
            "cd ..",
            'cmake -S . -B build -G "' .. cmake_gen .. '" -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_GPU=ON -DSDL_RENDER=OFF -DSDL_AUDIO=OFF -DSDL_CAMERA=OFF -DSDL_DISKAUDIO=OFF -DSDL_DUMMYAUDIO=OFF',
            "cmake --build build --config Release",
        }

    filter "configurations:Debug*"
        runtime "Debug"
        symbols "on"
        defines { "DEBUG", "_DEBUG" }
        targetsuffix "_d"

    filter "configurations:Development*"
        runtime "Release"
        symbols "on"
        optimize "on"
        defines { "DEBUG", "_DEBUG" }
        targetsuffix "_dev"

    filter "configurations:Release*"
        runtime "Release"
        optimize "on"