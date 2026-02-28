$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH
$logFile = "c:\Users\HP\.gemini\antigravity\scratch\pos-system\build_log.txt"

Push-Location "c:\Users\HP\.gemini\antigravity\scratch\pos-system\build"

Write-Output "=== CMAKE ===" | Out-File $logFile -Encoding UTF8
$cmakeOut = & cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/msys64/mingw64" .. 2>&1
$cmakeOut | Out-File $logFile -Append -Encoding UTF8
Write-Output "CMAKE_EXIT: $LASTEXITCODE" | Out-File $logFile -Append -Encoding UTF8

Write-Output "" | Out-File $logFile -Append -Encoding UTF8
Write-Output "=== MAKE ===" | Out-File $logFile -Append -Encoding UTF8
$makeOut = & mingw32-make -j4 2>&1
$makeOut | Out-File $logFile -Append -Encoding UTF8
Write-Output "MAKE_EXIT: $LASTEXITCODE" | Out-File $logFile -Append -Encoding UTF8

Pop-Location
Write-Output "Build complete. Check build_log.txt"
