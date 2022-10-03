project "hx7816graph"
    kind "ConsoleApp"
    language "C++"

    files {
        "**.h",
        "**.cpp",
    }

    links {
        "dl",
        "uv",
        "GL",
        "GLEW",
        "glut",
        "nanovg",
    }
