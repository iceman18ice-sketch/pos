@echo off
set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%
set LOGFILE=C:\Users\HP\.gemini\antigravity\scratch\pos-system\build_output.log

echo === CLEAN BUILD START %date% %time% === > "%LOGFILE%"

echo Cleaning build directory...
rd /s /q "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build" 2>nul
mkdir "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build"

echo Running CMake...
cd /d "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build"
cmake.exe -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/msys64/mingw64" -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/mingw32-make.exe" .. >> "%LOGFILE%" 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo CMAKE FAILED >> "%LOGFILE%"
    echo CMAKE FAILED - check build_output.log
    exit /b 1
)
echo CMAKE_EXIT=0 >> "%LOGFILE%"
echo CMake OK, starting compilation...

echo Building... >> "%LOGFILE%"
mingw32-make.exe -j4 >> "%LOGFILE%" 2>&1
set MAKE_EXIT=%ERRORLEVEL%
echo MAKE_EXIT=%MAKE_EXIT% >> "%LOGFILE%"

if %MAKE_EXIT% EQU 0 (
    echo BUILD SUCCESS
    echo === BUILD SUCCESS === >> "%LOGFILE%"
) else (
    echo BUILD FAILED - check build_output.log
    echo === BUILD FAILED === >> "%LOGFILE%"
)
exit /b %MAKE_EXIT%
