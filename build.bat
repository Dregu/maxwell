@echo off

:: build.bat [MSVC|Clang|Ninja] [Release|Debug|RelWithDebInfo] [custom options for cmake]

set A=Ninja
set B=Release
set C=""

if /I [%1] == [MSVC]  set A=MSVC
if /I [%1] == [Clang] set A=Clang
if /I [%1] == [Ninja] set A=Ninja

if /I [%2] == [Release]        set B=Release
if /I [%2] == [Debug]          set B=Debug
if /I [%2] == [RelWithDebInfo] set B=RelWithDebInfo

for /f "tokens=2,* delims= " %%a in ("%*") do set D=%%b
if [%D%] == [] (set C=%C%) else (set C=%D%)

echo Building %A% %B% with %C%

if ["%VSCMD_VER%"] == [""] call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat"

goto %A%

:MSVC
mkdir build
pushd build
set CMAKE_CXX_FLAGS=/MP
cmake .. -G"Visual Studio 17 2022" -A x64 %C%
cmake --build . --config %B%
goto end

:Clang
mkdir build-clang
pushd build-clang
set CMAKE_CXX_FLAGS=/MP
cmake .. -G"Visual Studio 17 2022" -TClangCL -A x64 %C%
cmake --build . --config %B%
goto end

:Ninja
mkdir build-ninja
pushd build-ninja
cmake .. -G"Ninja Multi-Config" %C%
cmake --build . --config %B%
goto end

:end
set A=
set B=
set C=
set D=
popd
