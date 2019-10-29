call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

cd %APPVEYOR_BUILD_FOLDER%
mkdir build
pushd build

set compiler_flags=-Od -MTd -nologo -fp:fast -fp:except- -EHsc -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4127 -wd4702 -wd4530 -wd4800 -FC -Z7 -I%APPVEYOR_BUILD_FOLDER%/src
set linker_flags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Shlwapi.lib Ws2_32.lib

cl %compiler_flags% %APPVEYOR_BUILD_FOLDER%/src/main.cpp -Fetempl.exe /link %linker_flags%

exit /b %ERRORLEVEL%
