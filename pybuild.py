from setuptools import setup, Extension

module = Extension(
    "KrooM",
    sources=["Py_Bind.cpp" , "src/Application.cpp" , "src/Shape.cpp", "src/Slider.cpp" ,"src/Button.cpp"],
    include_dirs=[
        r"vendor\glfw\include",
        r"vendor\stb",
        r"vendor\glew\include",
        r"vendor\python\include",
        r".\include"
    ],
    libraries=[
        "legacy_stdio_definitions",
        "msvcrt",
        "vcruntime",
        "ucrt",
        "python313",
        "glfw3",
        "glew32",
        "opengl32",
        "user32",
        "gdi32",
        "shell32",
        "winmm"
    ],
    library_dirs=[
        r"vendor\glfw\lib-vc2022",
        r"vendor\glew\lib\Release\x64",
        r"vendor\python\libs"
    ],
    extra_compile_args=["/std:c++20"],
)

setup(
    name="KrooM",
    version="1.0",
    description="UI Library For Som Krooz",
    ext_modules=[module],
)
