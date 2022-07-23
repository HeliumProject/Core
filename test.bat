@echo off

setlocal EnableDelayedExpansion

set CONFIG=%1
if "%CONFIG%" equ "" echo Please specify the build config.
if "%CONFIG%" equ "" exit /b 1

if not exist "%~dp0Bin\%CONFIG%" echo %CONFIG% isn't a valid config.
if not exist "%~dp0Bin\%CONFIG%" exit /b 1

set TESTS=^
PlatformTests.exe ^
FoundationTests.exe ^
MathTests.exe ^
ReflectTests.exe ^
PersistTests.exe ^
InspectTests.exe ^
ApplicationTests.exe ^
MongoTests.exe

for %%f in (%TESTS%) do (
    "%~dp0Bin\%CONFIG%\%%f"
    if errorlevel 1 goto FAIL
)

goto SUCCESS

:FAIL
endlocal
exit /b 1

:SUCCESS
endlocal
exit /b 0