include "dependencies.lua"

workspace "based"
    startproject "BasedEditor"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

tdir = "bin/%{cfg.buildcfg}/%{prj.name}"
odir = "bin-obj/%{cfg.buildcfg}/%{prj.name}"

group "Core"
    include "based"
group ""

group "Dependencies"
    include "external/glad"
group ""

group "Tools"
    include "BasedEditor"
group ""

group "Misc"
    include "Sandbox"
group ""
