#!/bin/bash
# scripts/run.sh
# Run this script to inject the built .dll into the running ZumbiBlocks2.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# For end user that just download from Release
if ls "$SCRIPT_DIR/"*.dll 1> /dev/null 2>&1 && [ -f "$SCRIPT_DIR/injector.exe" ]; then
    DLL_PATH=$(ls "$SCRIPT_DIR/"*.dll | head -n 1)
    INJECTOR_EXE="$SCRIPT_DIR/injector.exe"
# For user who build from source code
elif ls "$SCRIPT_DIR/../../build/release/"*.dll 1> /dev/null 2>&1 && [ -f "$SCRIPT_DIR/../../build/release/injector.exe" ]; then
    DLL_PATH=$(ls "$SCRIPT_DIR/../../build/release/"*.dll | head -n 1)
    INJECTOR_EXE="$SCRIPT_DIR/../../build/release/injector.exe"
else
    echo "Error: Could not find any .dll and injector.exe. Please build the project or extract the release archive."
    exit 1
fi

echo "Looking for ZumbiBlocks2.exe..."
PID=$(pgrep -f "ZumbiBlocks2.exe" | head -n 1)

if [ -z "$PID" ]; then
    echo "Error: Game is not running."
    exit 1
fi

echo "Game found at PID $PID."

# Extract STEAM_COMPAT_DATA_PATH from the process environment
COMPAT_DATA_PATH=$(strings /proc/$PID/environ | grep "^STEAM_COMPAT_DATA_PATH=" | cut -d= -f2-)
if [ -z "$COMPAT_DATA_PATH" ]; then
    echo "Error: Could not find STEAM_COMPAT_DATA_PATH for the game."
    exit 1
fi

# Extract STEAM_COMPAT_TOOL_PATHS
TOOL_PATHS=$(strings /proc/$PID/environ | grep "^STEAM_COMPAT_TOOL_PATHS=" | cut -d= -f2-)
PROTON_PATH=$(echo "$TOOL_PATHS" | cut -d: -f1)

if [ -z "$PROTON_PATH" ]; then
    echo "Error: Could not find Proton path for the game."
    exit 1
fi

PROTON_EXEC="$PROTON_PATH/proton"

if [ ! -f "$PROTON_EXEC" ]; then
    echo "Error: Proton executable not found at $PROTON_EXEC."
    exit 1
fi

# Convert Linux path to Wine "Z:" path
WINE_DLL_PATH="Z:${DLL_PATH}"
WINE_DLL_PATH="${WINE_DLL_PATH//\//\\}"

echo "STEAM_COMPAT_DATA_PATH: $COMPAT_DATA_PATH"
echo "Proton Path: $PROTON_EXEC"
echo "Injecting DLL: $WINE_DLL_PATH"

export STEAM_COMPAT_DATA_PATH="$COMPAT_DATA_PATH"

# Run the injector inside the game's prefix
"$PROTON_EXEC" runinprefix "$INJECTOR_EXE" "ZumbiBlocks2.exe" "$WINE_DLL_PATH"
