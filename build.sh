#!/bin/bash
set -e
export PATH="/mingw64/bin:$PATH"

PROJ="/c/Users/HP/.gemini/antigravity/scratch/pos-system"
BUILD="$PROJ/build"
LOG="$PROJ/bash_build.log"

echo "=== Clean Build Start $(date) ===" > "$LOG"

# Clean
rm -rf "$BUILD"
mkdir -p "$BUILD"
cd "$BUILD"

# CMake
echo "=== CMAKE ===" >> "$LOG"
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="/mingw64" -DCMAKE_MAKE_PROGRAM="/mingw64/bin/mingw32-make.exe" .. >> "$LOG" 2>&1
echo "CMAKE_EXIT=$?" >> "$LOG"

# Make
echo "=== MAKE ===" >> "$LOG"
mingw32-make -j4 >> "$LOG" 2>&1
MEXIT=$?
echo "MAKE_EXIT=$MEXIT" >> "$LOG"

if [ $MEXIT -eq 0 ]; then
    echo "=== BUILD SUCCESS ===" >> "$LOG"
    echo "BUILD SUCCESS"
else
    echo "=== BUILD FAILED ===" >> "$LOG"
    echo "BUILD FAILED"
fi
