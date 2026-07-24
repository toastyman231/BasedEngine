project "based"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    location "../Intermediate"

    targetdir(tdir)
    objdir(odir)

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    filter "action:ninja"
        enablepch "off"
        forceincludes { "pch.h" }
    filter {}

    EXTERNALS_DIR_PRIVATE = "../external/Private/"
    EXTERNALS_DIR_PUBLIC = "../external/Public/"

    files 
    {
        "include/**.h",
        "include/**.hpp",
        "src/**.h",
        "src/**.cpp",
        "**.natvis",
    }

    -- Not all of these are actually supported
    local systems = {
        "Windows",
        "Linux",
        "MacOSX",
        "PS5",
        "Xbox",
        "Android",
        "iOS",
        "emscripten"
    }

    -- Remove any platform specific files, we will add back just the ones for this platform later
    for _, sys in ipairs(systems) do
        removefiles 
        { 
            "include/based/**/" .. sys .. "/**.h",
            "include/based/**/" .. sys .. "/**.hpp",
            "src/**/" .. sys .. "/**.c",
            "src/**/" .. sys .. "/**.cpp",
        }
    end

    -- Remove graphics API specific files, we will add back just the ones for this platform's graphics API
    removefiles
    {
        "include/based/**/vulkan/**.h",
        "include/based/**/vulkan/**.hpp",
        "src/**/vulkan/**.c",
        "src/**/vulkan/**.cpp",
    }

    files
    {
        "include/**/%{cfg.system}/**.h",
        "include/**/%{cfg.system}/**.hpp",
        "src/**/%{cfg.system}/**.c",
        "src/**/%{cfg.system}/**.cpp",
    }

    includedirs
    {
        EXTERNALS_DIR_PUBLIC,
        "include/based",
        EXTERNALS_DIR_PRIVATE .. "assimp/include",
        EXTERNALS_DIR_PRIVATE .. "freetype/include",
        EXTERNALS_DIR_PRIVATE .. "jolt/include",
        EXTERNALS_DIR_PRIVATE .. "ktx-software/include",
        EXTERNALS_DIR_PRIVATE .. "rmlui/include",
        EXTERNALS_DIR_PRIVATE .. "sdl3/include",
        EXTERNALS_DIR_PRIVATE .. "spdlog/include",
        EXTERNALS_DIR_PRIVATE .. "tlsf/include",
        EXTERNALS_DIR_PRIVATE .. "tracy/public",
        EXTERNALS_DIR_PRIVATE .. "yaml-cpp/include",
    }

    defines
    {
        "RMLUI_STATIC_LIB",
        "KHRONOS_STATIC",
        "GLM_ENABLE_EXPERIMENTAL",
    }

    -- Graphics API
    filter "system:windows or linux or android"
        VULKAN_PRIOR_PROJECT = "based"
        include("" .. EXTERNALS_DIR_PRIVATE .. "VulkanSDK")
    filter {}

    filter {"system:windows", "configurations:*"}
        systemversion "latest"
        buildoptions "/bigobj"
    filter {}

    filter {"system:macosx", "configurations:*"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }
    filter {}

    filter {"system:windows or linux or macosx or emscripten", "configurations:*"}
        dependson { "sdl3_build" }
    filter {}

    filter "configurations:Debug*"
        defines { "JPH_DEBUG_RENDERER" }
        runtime "Debug"
        editandcontinue "off"
        symbols "on"
        optimize "debug"
        targetsuffix "_d"
    filter {}

    filter "configurations:Development*"
        defines { "JPH_DEBUG_RENDERER" }
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