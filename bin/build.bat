@echo off

set compiler_flags=-Od -MTd -nologo -DWEBC_LOAD_MODULES=1 -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4127 -wd4702 -FC -Z7 -I%PROJECT_PATH%/src -I%PROJECT_PATH%/lib -I%C_LIB_PATH%
set linker_flags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Shlwapi.lib Ws2_32.lib

IF NOT exist %BUILD_PATH%\build ( mkdir %BUILD_PATH%\build )
pushd %BUILD_PATH%\build

cl %compiler_flags% %PROJECT_PATH%\src\%PROJECT_MAIN_FILE% -Fetempl.exe /link %linker_flags%

popd
