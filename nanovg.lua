project "*"
    sysincludedirs
    {
        "nanovg/src",
    }

project "nanovg"
    language "C"
    kind "StaticLib"
    includedirs { "nanovg/src" }
    files { "nanovg/src/*.c" }