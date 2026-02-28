@echo off
chcp 65001 >nul
set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%

echo ========================================
echo    Build POS System
echo ========================================

cd /d "C:\Users\HP\.gemini\antigravity\scratch\pos-system"

echo Cleaning old build...
rd /s /q build 2>nul
mkdir build
cd build

echo Running CMake...
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 ..
if %errorlevel% neq 0 (
    echo CMAKE FAILED!
    pause
    exit /b 1
)

echo.
echo Compiling...
mingw32-make -j4
if %errorlevel% neq 0 (
    echo BUILD FAILED!
    pause
    exit /b 1
)

echo.
echo ========================================
echo    BUILD SUCCESSFUL!
echo    Run: build\POSSystem.exe
echo ========================================
pause
