@echo off

setlocal

echo [Script] OS information.
powershell Get-WmiObject Win32_OperatingSystem
if %errorlevel% neq 0 exit /b %errorlevel%
echo.

echo [Script] CMake information.
cmake --version
if %errorlevel% neq 0 exit /b %errorlevel%
echo.

echo [Script] NMake information.
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
  set VCVARSALL=%%i\VC\Auxiliary\Build\vcvarsall.bat
)
call "%VCVARSALL%" x64
if %errorlevel% neq 0 exit /b %errorlevel%
nmake /?
if %errorlevel% neq 0 exit /b %errorlevel%
