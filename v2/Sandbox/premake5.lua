include "utils.lua"

ENGINE_DIR, SUCCESS, VAL = Utils.GetEngineInstallDir()
if not SUCCESS then print("Error getting engine directory: " .. VAL) end

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    location "Intermediate"
    targetdir(tdir)
    debugdir(tdir)
    objdir(odir)

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

    local build_tool = path.getabsolute(ENGINE_DIR .. "/tools/basedbuildtool.py")
    build_tool = build_tool:gsub("\\", "/")

    postbuildcommands
    {
        "python3 " .. build_tool .. " -i " .. path.getabsolute(".") .. " -c %{cfg.buildcfg} -q 0.05 --ci",
    }

    filter "system:emscripten"
        local postbuild_dir = path.getabsolute("bin-obj/%{cfg.system}/%{cfg.buildcfg}/%{prj.name}")
        postbuild_dir = postbuild_dir:gsub("\\", "/")

        os.mkdir(postbuild_dir)

        local launch_tool = path.getabsolute(ENGINE_DIR .. "/tools/launch_web.py")
        launch_tool = launch_tool:gsub("\\", "/")

        local output_dir = path.getabsolute("bin/%{cfg.system}/%{cfg.buildcfg}/%{prj.name}")
        output_dir = output_dir:gsub("\\", "/")

        local patch_tool = path.getabsolute(ENGINE_DIR .. "/tools/patch_js.py")
        patch_tool = patch_tool:gsub("\\", "/")

        local js_output = path.getabsolute(tdir .. "/%{cfg.buildtarget.basename}.js")
        js_output = js_output:gsub("\\", "/")

        postbuildcommands
        {
            "python3 " .. launch_tool .. " " .. output_dir,
            "python3 " .. patch_tool .. " " .. js_output
        }
    filter{}

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        --files { "resources.rc", "Assets/**.ico" }
        --vpaths { ['Assets/*'] = { '*.rc', '**.ico' } }

        defines { "JPH_DEBUG_RENDERER" }

        links
        {
            "Winmm",     -- Fixes timeBeginPeriod / timeEndPeriod
            "Setupapi",  -- Fixes SetupDiGetClassDevsA / SetupDiGetDeviceInstanceIdA
            "Version",   -- Required for SDL Windows version checks
            "Imm32"      -- Required for SDL Input Method Editor (IME) support
        }
    filter {}

    filter {"system:windows or macosx", "configurations:Release*"}
        kind "WindowedApp"
    filter {}

    local function align_to_64k(value)
        return math.floor((value + 65535) / 65536) * 65536
    end

    local stack_size = 1 * 1024 * 1024

    local initial_mem = align_to_64k(2 * stack_size) -- We need SOME initial memory or we crash.
    local max_mem = 17179869184 -- This is the max that emscripten will even allow

    filter { "system:windows or macosx or linux", "not platforms:Web" }
        libdirs { ENGINE_DIR .. "/external/Private/sdl3/build-%{cfg.system}/%{cfg.buildcfg:find('^Debug') and 'Debug' or (cfg.buildcfg:find('^Development') and 'RelWithDebInfo' or 'Release')}" }
        links { "%{(cfg.system == 'windows') and 'SDL3-static' or 'SDL3'}" }
    filter {}

    filter "system:emscripten"
        defines 
        {
            "SKIP_AVAILABLE_MEMORY_CHECK"
        }
        linkoptions 
        {
            path.getabsolute(ENGINE_DIR .. "/external/Private/sdl3/build-emscripten/libSDL3.a"),
            "-sUSE_SDL=3",
            "-sALLOW_MEMORY_GROWTH=1",
            "-sASYNCIFY",
            "-sSTACK_SIZE=" .. stack_size,
            "-sINITIAL_MEMORY=" .. initial_mem,
            "-sMAXIMUM_MEMORY=" .. max_mem
        }
    filter {}

    filter { "system:emscripten", "configurations:Debug* or Development*" }
        linkoptions
        {
            "-g2" -- Required for debug stacks
        }
    filter {}
    
    filter "configurations:Debug*"
        runtime "Debug"
        editandcontinue "off"
        symbols "on"
        optimize "debug"
        targetsuffix "_d"
    filter {}

    filter "configurations:Development*"
        runtime "Release"
        editandcontinue "off"
        optimize "on"
        symbols "on"
        targetsuffix "_dev"
    filter {}

    filter "configurations:Release*"
        fatalwarnings "All"
        runtime "Release"
        symbols "off"
        optimize "on"
    filter {}