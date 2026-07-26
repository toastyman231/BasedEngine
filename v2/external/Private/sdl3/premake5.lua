project "sdl3_build"
    kind "StaticLib"
    language "C"

    local dummy_output = path.getabsolute("Intermediate/sdl3_dummy_output")
    local dummy_stamp_dir = dummy_output .. "/%{cfg.system}/%{cfg.buildcfg}"
    local dummy_stamp = dummy_stamp_dir .. "/test.txt"

    location(path.getabsolute("Intermediate"))
    targetdir(dummy_output .. "/%{cfg.system}/%{cfg.buildcfg}")
    objdir(dummy_output .. "/%{cfg.system}/%{cfg.buildcfg}")

    files {
        "../sdl3_dummy/SDL3_Dummy.c"
    }

    local script = path.getabsolute("../sdl3_dummy/build_sdl3.py")
    script = script:gsub("\\", "/")

    local python_cmd = GetPythonCommand()

    filter "configurations:Debug*"
        prebuildcommands {
            python_cmd .. " " .. script .. " Debug %{cfg.system} && if not exist " .. dummy_stamp_dir .. " mkdir " .. dummy_stamp_dir .. " && type nul > " .. dummy_stamp
        }
    filter {}

    filter "configurations:Development*"
        prebuildcommands {
            python_cmd .. " " .. script .. " RelWithDebInfo %{cfg.system} && if not exist " .. dummy_stamp_dir .. " mkdir " .. dummy_stamp_dir .. " && type nul > " .. dummy_stamp
        }
    filter {}

    filter "configurations:Release*"
        prebuildcommands {
            python_cmd .. " " .. script .. " Release %{cfg.system} && if not exist " .. dummy_stamp_dir .. " mkdir " .. dummy_stamp_dir .. " && type nul > " .. dummy_stamp
        }
    filter {}