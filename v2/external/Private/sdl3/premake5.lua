project "sdl3"
    kind "StaticLib"
    language "C"
    location "Intermediate"
    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-obj/%{cfg.buildcfg}/%{prj.name}"

    files
    {
        "include/**.h",
        "src/*.c",
        "src/*.h",
        "src/atomic/*.c",
        "src/atomic/*.h",
        "src/audio/*.c",
        "src/audio/*.h",
        "src/audio/dummy/*.c",
        "src/audio/dummy/*.h",
        "src/camera/*.c",
        "src/camera/*.h",
        "src/camera/dummy/*.c",
        "src/camera/dummy/*.h",
        "src/core/*.c",
        "src/core/*.h",
        "src/cpuinfo/*.c",
        "src/cpuinfo/*.h",
        "src/dialog/*.c",
        "src/dialog/*.h",
        "src/dynapi/*.c",
        "src/dynapi/*.h",
        "src/events/*.c",
        "src/events/*.h",
        "src/filesystem/*.c",
        "src/filesystem/*.h",
        "src/gpu/*.c",
        "src/gpu/*.h",
        "src/haptic/*.c",
        "src/haptic/*.h",
        "src/haptic/dummy/*.c",
        "src/haptic/dummy/*.h",
        "src/hidapi/*.c",
        "src/hidapi/*.h",
        "src/io/*.c",
        "src/io/*.h",
        "src/io/generic/*.c",
        "src/io/generic/*.h",
        "src/joystick/*.c",
        "src/joystick/*.h",
        "src/joystick/dummy/*.c",
        "src/joystick/dummy/*.h",
        "src/joystick/virtual/*.c",
        "src/joystick/virtual/*.h",
        "src/libm/*.c",
        "src/libm/*.h",
        "src/loadso/dummy/*.c",
        "src/locale/*.c",
        "src/locale/*.h",
        "src/locale/dummy/*.c",
        "src/main/*.c",
        "src/main/*.h",
        "src/main/generic/*.c",
        "src/main/generic/*.h",
        "src/misc/*.c",
        "src/misc/*.h",
        "src/misc/dummy/*.c",
        "src/power/*.c",
        "src/power/*.h",
        "src/process/*.c",
        "src/process/*.h",
        "src/process/dummy/*.c",
        "src/render/*.c",
        "src/render/*.h",
        "src/render/software/*.c",
        "src/render/software/*.h",
        "src/sensor/*.c",
        "src/sensor/*.h",
        "src/sensor/dummy/*.c",
        "src/sensor/dummy/*.h",
        "src/stdlib/*.c",
        "src/stdlib/*.h",
        "src/storage/*.c",
        "src/storage/*.h",
        "src/storage/generic/*.c",
        "src/storage/generic/*.h",
        "src/thread/*.c",
        "src/thread/*.h",
        "src/time/*.c",
        "src/time/*.h",
        "src/timer/*.c",
        "src/timer/*.h",
        "src/tray/*.c",
        "src/tray/*.h",
        "src/tray/dummy/*.c",
        "src/video/*.c",
        "src/video/*.h",
        "src/video/dummy/*.c",
        "src/video/dummy/*.h",
        "src/video/offscreen/*.c",
        "src/video/offscreen/*.h",
        "src/video/yuv2rgb/*.c",
        "src/video/yuv2rgb/*.h",
        "src/video/khronos/**.h",
    }

    includedirs
    {
        "include",
        "src",
        "src/video/khronos",
        -- Generated headers - run CMake once to produce these
        "build/include-config-debug/build_config",
        "build/include-revision",
    }

    defines
    {
        "SDL_STATIC_LIB",
        "USING_GENERATED_CONFIG_H",
        "SDL_BUILD_MAJOR_VERSION=3",
        "SDL_BUILD_MINOR_VERSION=4",
        "SDL_BUILD_MICRO_VERSION=10",
    }

    enablepch "off"

    -- Windows
    filter "system:windows"
        systemversion "latest"
        staticruntime "off" -- MUST BE OFF FOR MIMALLOC-REDIRECT TO WORK!
        linkoptions { "/ignore:4006" }
        defines
        {
            "_CRT_SECURE_NO_DEPRECATE",
            "_CRT_NONSTDC_NO_DEPRECATE",
            "_CRT_SECURE_NO_WARNINGS",
            "WIN32_LEAN_AND_MEAN",
        }
        files
        {
            "src/core/windows/*.c",
            "src/core/windows/*.cpp",
            "src/core/windows/*.h",
            "src/audio/directsound/*.c",
            "src/audio/directsound/*.h",
            "src/audio/wasapi/*.c",
            "src/audio/wasapi/*.h",
            "src/dialog/windows/*.c",
            "src/filesystem/windows/*.c",
            "src/gpu/d3d12/*.c",
            "src/gpu/d3d12/*.h",
            "src/haptic/windows/*.c",
            "src/haptic/windows/*.h",
            "src/joystick/windows/*.c",
            "src/joystick/windows/*.h",
            "src/joystick/gdk/*.cpp",
            "src/loadso/windows/*.c",
            "src/locale/windows/*.c",
            "src/main/windows/*.c",
            "src/misc/windows/*.c",
            "src/power/windows/*.c",
            "src/process/windows/*.c",
            "src/process/windows/*.h",
            "src/render/direct3d/*.c",
            "src/render/direct3d/*.h",
            "src/render/direct3d11/*.c",
            "src/render/direct3d11/*.h",
            "src/render/direct3d12/*.c",
            "src/render/direct3d12/*.h",
            "src/render/vulkan/*.c",
            "src/render/vulkan/*.h",
            "src/sensor/windows/*.c",
            "src/sensor/windows/*.h",
            "src/storage/steam/*.c",
            "src/storage/steam/*.h",
            "src/thread/generic/SDL_syscond.c",
            "src/thread/generic/SDL_syscond_c.h",
            "src/thread/generic/SDL_sysrwlock.c",
            "src/thread/generic/SDL_sysrwlock_c.h",
            "src/thread/windows/*.c",
            "src/thread/windows/*.h",
            "src/time/windows/*.c",
            "src/timer/windows/*.c",
            "src/tray/windows/*.c",
            "src/video/windows/*.c",
            "src/video/windows/*.h",
            "src/io/windows/*.c",
            "src/filesystem/posix/SDL_sysfsops.c",
        }
        links
        {
            "kernel32", "user32", "gdi32", "winmm",
            "imm32", "ole32", "oleaut32", "version",
            "uuid", "advapi32", "setupapi", "shell32",
            "dinput8", "dxguid",
        }

    -- Linux
    filter "system:linux"
        pic "On"
        systemversion "latest"
        files
        {
            "src/core/unix/*.c",
            "src/core/unix/*.h",
            "src/core/linux/*.c",
            "src/core/linux/*.h",
            "src/audio/alsa/*.c",
            "src/audio/alsa/*.h",
            "src/audio/pulseaudio/*.c",
            "src/audio/pulseaudio/*.h",
            "src/audio/pipewire/*.c",
            "src/audio/pipewire/*.h",
            "src/camera/v4l2/*.c",
            "src/camera/v4l2/*.h",
            "src/dialog/unix/*.c",
            "src/dialog/unix/*.h",
            "src/filesystem/unix/*.c",
            "src/filesystem/posix/SDL_sysfsops.c",
            "src/gpu/vulkan/*.c",
            "src/gpu/vulkan/*.h",
            "src/haptic/linux/*.c",
            "src/haptic/linux/*.h",
            "src/joystick/linux/*.c",
            "src/joystick/linux/*.h",
            "src/joystick/steam/*.c",
            "src/joystick/steam/*.h",
            "src/loadso/dlopen/*.c",
            "src/locale/unix/*.c",
            "src/misc/unix/*.c",
            "src/power/linux/*.c",
            "src/process/posix/*.c",
            "src/process/posix/*.h",
            "src/render/vulkan/*.c",
            "src/render/vulkan/*.h",
            "src/sensor/dummy/*.c",
            "src/storage/steam/*.c",
            "src/storage/steam/*.h",
            "src/thread/pthread/*.c",
            "src/thread/pthread/*.h",
            "src/time/unix/*.c",
            "src/timer/unix/*.c",
            "src/tray/unix/*.c",
            "src/video/x11/*.c",
            "src/video/x11/*.h",
            "src/video/wayland/*.c",
            "src/video/wayland/*.h",
        }
        links { "dl", "pthread", "m" }

    -- macOS
    filter "system:macosx"
        pic "On"
        systemversion "latest"
        files
        {
            "src/core/unix/*.c",
            "src/core/unix/*.h",
            "src/audio/coreaudio/*.m",
            "src/audio/coreaudio/*.h",
            "src/camera/coremedia/*.m",
            "src/dialog/cocoa/*.m",
            "src/filesystem/cocoa/*.m",
            "src/filesystem/posix/SDL_sysfsops.c",
            "src/gpu/metal/*.m",
            "src/gpu/metal/*.h",
            "src/gpu/vulkan/*.c",
            "src/gpu/vulkan/*.h",
            "src/haptic/darwin/*.c",
            "src/haptic/darwin/*.h",
            "src/joystick/apple/*.m",
            "src/joystick/apple/*.h",
            "src/joystick/darwin/*.c",
            "src/joystick/darwin/*.h",
            "src/loadso/dlopen/*.c",
            "src/locale/macos/*.m",
            "src/misc/macos/*.m",
            "src/power/macos/*.c",
            "src/process/posix/*.c",
            "src/process/posix/*.h",
            "src/render/metal/*.m",
            "src/render/metal/*.h",
            "src/render/vulkan/*.c",
            "src/render/vulkan/*.h",
            "src/storage/steam/*.c",
            "src/storage/steam/*.h",
            "src/thread/pthread/*.c",
            "src/thread/pthread/*.h",
            "src/time/unix/*.c",
            "src/timer/unix/*.c",
            "src/tray/cocoa/*.m",
            "src/video/cocoa/*.m",
            "src/video/cocoa/*.h",
        }
        links
        {
            "dl", "pthread", "m",
        }
        linkoptions
        {
            "-framework Cocoa",
            "-framework Carbon",
            "-framework IOKit",
            "-framework CoreAudio",
            "-framework AudioToolbox",
            "-framework AVFoundation",
            "-framework CoreVideo",
            "-framework CoreMedia",
            "-framework Metal",
            "-framework QuartzCore",
            "-framework GameController",
            "-framework CoreHaptics",
            "-framework Foundation",
            "-framework ForceFeedback",
            "-weak_framework UniformTypeIdentifiers",
            "-weak_framework CoreHaptics",
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