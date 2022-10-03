workspace "hx7816graph"
    configurations { "Debug", "Release"}
    targetdir "bin/%{cfg.buildcfg}"
    location "build"

    filter "system:windows"
        defines {
            "_WIN32",
            "_WIN32_WINNT=0x0601",
            "_CRT_SECURE_NO_WARNINGS",
        }

    filter "system:linux"
        toolset "gcc"
        buildoptions { "-fPIC" }

    filter { "system:not windows", "language:C++" }
        buildoptions { "-std=c++11" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "On"

    include "nanovg"
    include "hx7816graph"

