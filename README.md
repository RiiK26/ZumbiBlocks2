# Zumbi Blocks 2 Internal Mod

An internal mod and trainer menu for **Zumbi Blocks 2 (Open Alpha)**, built in C++23 with an ImGui overlay supporting both DirectX 11 and DirectX 12 renderers. Compatible with native Windows and Linux (Steam Proton / Steam Deck).

## Features

### Player
* **Invincible (God Mode)**: Prevents damage and keeps health topped up, preserving survival against lethal attacks.
* **Infinite Stamina**: Freezes stamina consumption for unlimited sprinting, leaping, and dodging.
* **Movement Speed Editor**: Customizable movement speed multiplier (1.0x - 10.0x) that dynamically scales walk, sprint, and swimming speeds.
* **No Clip (Fly)**: 3D flight mode with colliders and gravity disabled—pass freely through walls, ceilings, gates, and terrain with full camera direction control (`W/S/A/D`, `Space` to ascend, `Ctrl/C` to descend, `Shift` to boost).
* **Unlock All Loadouts & Skins**: Instantly unlocks all weapons, attachments, character skins, accessories, and perk slots, with persistent saving to disk.

### Combat
* **One-Hit Kill**: Multiplies firearm, melee weapon, and unarmed bare-fist punch damage to eliminate zombies instantly.
* **Infinite Ammo**: Prevents gun clip and magazine depletion—fire continuously without reloading.
* **Aura Kill**: Automatically eliminates all zombies entering your proximity aura (~15m). Accurately credits your personal kill count, triggers kill audio effects, and progresses kill challenges without glitching zombie health.

### Visuals
* **Custom Crosshair Overlay**: Screen-centered crosshair overlay with configurable styles (Standard Crosshair or Dot Crosshair) rendered through ImGui.

### Config & System
* **Config Persistence**: Save and reload your cheat configurations via `config.txt`.
* **Safe Unload**: Cleanly unhooks all DirectX hooks, restores game state, and frees the injected module.

---

## Controls

| Hotkey | Action |
| :--- | :--- |
| **`[INSERT]`** | Toggle Mod Menu UI |
| **`[END]`** | Safely unhook and unload mod |

---

## Getting Started

### Option 1: Download from Releases
1. Download the latest `ZumbiBlocks2-<version>.zip` from [Releases](https://github.com/RiiK26/ZumbiBlocks2/releases).
2. Extract the archive.

#### On Linux (Steam Proton / Steam Deck):
1. Start **Zumbi Blocks 2** via Steam.
2. In the terminal, execute the bundled runner script:
   ```bash
   ./run.sh
   ```

#### On Windows:
1. Start **Zumbi Blocks 2**.
2. Inject `ZumbiBlocks2.dll` into `ZumbiBlocks2.exe` using any 64-bit DLL injector (e.g., Xenos, Cheat Engine).

---

### Option 2: Build from Source

#### Prerequisites (Linux / Cross-compiling)
* GCC / G++ MinGW-w64 cross-compiler (`x86_64-w64-mingw32-gcc`, `x86_64-w64-mingw32-g++`)
* CMake (>= 3.24)
* Make or Ninja

On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 cmake make
```

#### Compile
Use the built-in build script:
```bash
# Compile Release (default)
./resources/scripts/build --release

# Compile Debug
./resources/scripts/build --debug

# Clean build directory and recompile
./resources/scripts/build --clean --release
```

The compiled binary will be placed at:
```
build/release/ZumbiBlocks2.dll
```

#### Run / Inject (Linux)
```bash
./resources/scripts/run.sh
```

---

## Tech Stack & Architecture

* **Language**: C++23 (`-std=c++23`)
* **Build System**: CMake cross-compiling via MinGW-w64
* **GUI / Overlay**: Dear ImGui with dual DX11 / DX12 backend hooks
* **Hooking**: MinHook (dynamic API redirection)
* **Game Engine Interop**: Dynamic Mono runtime resolver (`mono-2.0-bdwgc.dll`) without hardcoded memory offsets

---

## License

This project licensed under [MIT License](license).
