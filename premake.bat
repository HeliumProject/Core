@echo off

setlocal enabledelayedexpansion

set VSVERSION=
for /f "delims=" %%f in ( '"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property catalog_productLineVersion' ) do (
    if "!VSVERSION!" equ "" set VSVERSION=%%f
)

set VSINSTALL=
for /f "delims=" %%f in ( '"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath' ) do (
    if "!VSINSTALL!" equ "" set VSINSTALL=%%f
)

echo:
echo VSVERSION = %VSVERSION%
echo VSINSTALL = %VSINSTALL%

echo:
echo Setting up visual studio environment...
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"

echo:
echo Checking for existing premake...
pushd %~dp0Dependencies\premake
if not exist bin\release\premake5.exe nmake -f Bootstrap.mak MSDEV=vs%VSVERSION% windows-msbuild
popd

echo:
echo Running premake...
"%~dp0Dependencies\premake\bin\release\premake5.exe" %*

endlocal