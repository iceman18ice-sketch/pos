$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH
$logPath = "C:\Users\HP\.gemini\antigravity\scratch\pos-system\ps_build_log.txt"

"=== BUILD START $(Get-Date) ===" | Out-File $logPath -Encoding ASCII

# Clean
if (Test-Path "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build") {
    Remove-Item -Recurse -Force "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build"
}
New-Item -ItemType Directory "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build" -Force | Out-Null

# CMake
Set-Location "C:\Users\HP\.gemini\antigravity\scratch\pos-system\build"
"=== RUNNING CMAKE ===" | Out-File $logPath -Append -Encoding ASCII
$cmakeResult = & cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/msys64/mingw64" .. 2>&1
$cmakeResult | Out-File $logPath -Append -Encoding ASCII
"CMAKE_EXIT=$LASTEXITCODE" | Out-File $logPath -Append -Encoding ASCII

# Make
"=== RUNNING MAKE ===" | Out-File $logPath -Append -Encoding ASCII
$makeResult = & mingw32-make -j4 2>&1
$makeResult | Out-File $logPath -Append -Encoding ASCII
"MAKE_EXIT=$LASTEXITCODE" | Out-File $logPath -Append -Encoding ASCII
"=== BUILD END ===" | Out-File $logPath -Append -Encoding ASCII
