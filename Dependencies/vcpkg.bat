@echo off

setlocal

set VSVERSION=%1
if "%VSVERSION%" equ "" echo Please specify the vstudio version. For example: %~n0 vs2019
if "%VSVERSION%" equ "" goto FAIL

set VCPKG_INSTALLED=vcpkg-installed

set VCPKG_TRIPLETS=vcpkg-windows-%VSVERSION%

set VCPKG_PORTS=glfw3 gtest imgui libbson mongo-c-driver rapidjson zlib

echo:
echo === Bootstrapping vcpkg ===
:: -disableMetrics in important to avoid Malwarebytes quarantine the vcpkg file. 
call "%~dp0vcpkg\bootstrap-vcpkg.bat" -disableMetrics

:: build for each triplet
for %%x in (%VCPKG_TRIPLETS%) do (
    echo:
    echo === Running vcpkg for %%x ===
    "%~dp0vcpkg\vcpkg.exe" install --x-install-root="%~dp0%VCPKG_INSTALLED%" --overlay-triplets=./ --triplet=%%x %VCPKG_PORTS%
    if ERRORLEVEL 1 exit /b 1
)

endlocal
exit /b 0

:FAIL
endlocal
exit /b 1