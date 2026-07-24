-- Include this inside the project you want to add Vulkan to!
-- It will add the includedirs to that project, then define this separate project
-- for the actual compilation step.

VULKAN_SDK_VER = "1.4.341.1"

project(VULKAN_PRIOR_PROJECT) -- Make sure this is defined inside the project you want to return to after this
    includedirs
    {
        VULKAN_SDK_VER .. "/Include/SPIRV-Reflect",
        VULKAN_SDK_VER .. "/Include/Volk",
        VULKAN_SDK_VER .. "/Include",
    }

    files
    {
        "../../../based/include/based/**/vulkan/**.h",
        "../../../based/include/based/**/vulkan/**.hpp",
        "../../../based/src/**/vulkan/**.c",
        "../../../based/src/**/vulkan/**.cpp",
    }

    defines
    {
        "BASED_USE_VULKAN",
        "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1",
        "VK_NO_PROTOTYPES"
    }

    links { "VulkanLibs" }

project "VulkanLibs"
    kind "StaticLib"
    language "C"
    location "Intermediate"

    files
    {
        VULKAN_SDK_VER .. "/Source/**.c",
    }

    includedirs
    {
        VULKAN_SDK_VER .. "/Include/SPIRV-Reflect",
        VULKAN_SDK_VER .. "/Include/Volk",
        VULKAN_SDK_VER .. "/Include",
    }

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

project(VULKAN_PRIOR_PROJECT)