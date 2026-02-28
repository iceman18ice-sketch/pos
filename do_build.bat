@echo off
set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%
set LOGFILE=C:\Users\HP\.gemini\antigravity\scratch\pos-system\buildlog.txt

echo ====== BUILD START ====== > %LOGFILE%
echo [Step 1] Cleaning build... >> %LOGFILE%
rd /s /q "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build" 2>>%LOGFILE%
mkdir "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build" 2>>%LOGFILE%

echo [Step 2] CMake Configure... >> %LOGFILE%
cd /d "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build"
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 .. >>%LOGFILE% 2>&1
echo CMAKE_EXIT=%errorlevel% >> %LOGFILE%

echo [Step 3] Building... >> %LOGFILE%
mingw32-make -j4 >>%LOGFILE% 2>&1
echo MAKE_EXIT=%errorlevel% >> %LOGFILE%

echo ====== BUILD END ====== >> %LOGFILE%
