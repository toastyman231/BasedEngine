-- External Dependencies
externals = {}
externals["maclibs"] = (engineLocation or "%{wks.location}") .. "/external/maclibs"
externals["sdl2"] = (engineLocation or "%{wks.location}") .. "/external/sdl2/include"
externals["spdlog"] = (engineLocation or "%{wks.location}") .. "/external/spdlog/include"
externals["glad"] = (engineLocation or "%{wks.location}") .. "/external/glad/include"
externals["freetype"] = (engineLocation or "%{wks.location}") .. "/external/freetype/include"
externals["miniaudio"] = (engineLocation or "%{wks.location}") .. "/external/miniaudio"
externals["assimp"] = (engineLocation or "%{wks.location}") .. "/external/assimp/include"
externals["rmlui"] = (engineLocation or "%{wks.location}") .. "/external/rmlui/Include"
externals["tracy"] = (engineLocation or "%{wks.location}") .. "/external/tracy"
externals["yaml_cpp"] = (engineLocation or "%{wks.location}") .. "/external/yaml-cpp/include"
externals["jolt"] = (engineLocation or "%{wks.location}") .. "/external/jolt/include"

-- External libraries
libraries = {}
libraries["maclibs"] = (engineLocation or "%{wks.location}") .. "/external/maclibs"
libraries["sdl2"] = (engineLocation or "%{wks.location}") .. "/external/sdl2/lib"
libraries["spdlog"] = (engineLocation or "%{wks.location}") .. "/external/spdlog/lib"
libraries["freetype"] = (engineLocation or "%{wks.location}") .. "/external/freetype/lib"
libraries["assimp"] = (engineLocation or "%{wks.location}") .. "/external/assimp/lib"
libraries["rmlui"] = (engineLocation or "%{wks.location}") .. "/external/rmlui/lib"
libraries["jolt"] = (engineLocation or "%{wks.location}") .. "/external/jolt/lib/%{cfg.buildcfg}"