#!/bin/bash
# Radio App — macOS quick-launch script
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BINARY="$BUILD_DIR/radio-app-gui"

# Build if binary doesn't exist or source is newer
if [ ! -f "$BINARY" ] || find "$SCRIPT_DIR/src" "$SCRIPT_DIR/qml" -newer "$BINARY" | grep -q .; then
    echo "Building Radio App..."
    cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$BUILD_DIR" --target radio-app-gui -j"$(sysctl -n hw.ncpu)"
fi

exec "$BINARY"
