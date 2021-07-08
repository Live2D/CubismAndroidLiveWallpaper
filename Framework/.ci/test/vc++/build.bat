@echo off

setlocal

rem Initialize Visual Studio toolchain.
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
  set VCVARSALL=%%i\VC\Auxiliary\Build\vcvarsall.bat
)
call "%VCVARSALL%" x64
if %errorlevel% neq 0 exit /b %errorlevel%

cmake -S . -B build -G "NMake Makefiles" -D CMAKE_BUILD_TYPE="Release"
if %errorlevel% neq 0 exit /b %errorlevel%
cd build && nmake
if %errorlevel% neq 0 exit /b %errorlevel%
