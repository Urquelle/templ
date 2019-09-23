@echo off

SET PROJECT_NAME=templ
SET PROJECT_MAIN_FILE=main.cpp
SET PROJECT_LINKER_FLAGS=user32.lib gdi32.lib winmm.lib Shlwapi.lib

SET PATH=C:\Users\serge\Dev\Cpp\bin;%PATH%

call global_shell.bat
