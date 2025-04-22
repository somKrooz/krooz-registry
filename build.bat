
@echo off
setlocal

REM Output executable
set OUT=Krooz.exe

REM Source file
set SRC=main.cpp

REM Check if we're in a developer command prompt
where cl >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Visual Studio compiler not found in PATH.
    echo Please run this script from a Visual Studio Developer Command Prompt,
    echo or use the 'x64 Native Tools Command Prompt for VS 2022' shortcut.
    exit /b 1
)

REM Compiler and flags - using environment variables already set by VS Command Prompt
set CFLAGS=/EHsc /W4 /std:c++17 /MD /I"./vendor/glfw/include" /I"./vendor/glew/include" /I"./include" /I"./vendor/glm"

REM Libraries to link
set LIBS=opengl32.lib glew32.lib glfw3.lib user32.lib gdi32.lib shell32.lib

REM Library paths
set LIBPATHS=/LIBPATH:"./vendor/glew/lib/Release/x64" /LIBPATH:"./vendor/glfw/lib-vc2022"

echo Building %OUT%...

REM Build the project
cl %CFLAGS% /Fe%OUT% %SRC% /link %LIBPATHS% %LIBS%

IF %ERRORLEVEL% NEQ 0 (
    echo Build Failed with errors.
) ELSE (
    echo Build Successfully Done!
    echo Run the application with: %OUT%
)
