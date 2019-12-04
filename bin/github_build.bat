mkdir build
pushd build

set compiler_flags=-Od -MTd -nologo -fp:fast -fp:except- -EHsc -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4127 -wd4702 -wd4530 -wd4800 -FC -Z7 -I../src
set linker_flags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Shlwapi.lib Ws2_32.lib
cl %compiler_flags% ..\examples\main.cpp -Fetempl.exe /link %linker_flags%

if %ERRORLEVEL% NEQ 0 (
    echo "fehler bei der kompilierung des projekts"
    exit /b %ERRORLEVEL%
)

popd
