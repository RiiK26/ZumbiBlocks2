# Project: ZumbiBlocks2

* Game Directory: `/data/SteamLibrary/steamapps/common/Zumbi Blocks 2 Open Alpha`
* Language: C#
* Game Engine: Unity
* Framework: Mono
* Dll Directory: [`/data/SteamLibrary/steamapps/common/Zumbi Blocks 2 Open Alpha/ZumbiBlocks2_Data/Managed/`, `/data/SteamLibrary/steamapps/common/Zumbi Blocks 2 Open Alpha/MonoBleedingEdge/EmbedRuntime/`]
* Dll: [`Assembly-CSharp.dll`, `mono-2.0-bdwgc.dll`]
* Platform: Linux & Windows
* Architecture: x64
* Graphics API: D3D12 (NATIVE)

## Tech Stack

* Language: C/C++ 23+
* Build System: CMake (MinGW Cross-Compiler for Windows targets on Linux)
* Build Type: default `Release`, available `Debug`
* GUI: ImGui (Vendor)
* Hook: MinHook (Vendor)

## Commands

- **Build Project**: `./resources/scripts/build` (Auto-compiles using `cmake/mingw-toolchain.cmake`)


## MCP listed Available

* ILSpy (static analysis)
* DLLStalker (runtime analysis)

## Code Conventions

- **Modular Architecture**:
  - `src/features/<Domain>/`: 1 Feature = 1 `.cpp` (e.g., `src/features/Combat/GodMode.cpp`). Orchestrator headers `.hpp` live inside each domain folder (e.g., `src/features/Combat/combat.hpp`). **No god files.**
  - `src/modules/`: Core infrastructure and major subsystems (e.g., `ui` for ImGui integration, hooking frameworks).
  - `src/modules/utils/`: Lightweight helpers and wrappers (e.g., `mono_resolver`).
- **UI/Feature Sync**: The `src/features/<Domain>/` directory structure MUST map exactly 1:1 with the ImGui Tab names defined in `Menu.cpp` (e.g., `Combat`, `Player`). If a feature is displayed in the "Player" tab, its source code MUST reside in `src/features/Player/`.
- **Hooking**: Always prefer AOB pattern scanning or Mono resolver hooking over static offsets where possible. Dynamic resolving and signatures are resilient to game updates and obfuscation, whereas hardcoded offsets break immediately.
- **Toggles & Config**: Store feature toggles and parameters in `Menu::ConfigData`. Load/save these values using `config.txt` at the project binaries directory. Avoid hardcoding magic numbers for features.

## Boundaries

- Do not use hardcoded process/module offsets when Mono method resolution or AOB scanning is available.
- Do not treat a UI value hook as a persistent state change; use request flags and original setters for one-time mutations.
- Do not add dependencies or alter game data/configuration without checking the existing build and runtime path.
- Do not expose secrets, credentials, or private runtime data in logs or documentation.

## Game Knowledge

### Player Structure
The player in Zumbi Blocks 2 is represented primarily by the `PlayerMain` class. Key fields and properties include:
- `HealthState healthState`: Enum tracking if the player is `Alive`, `Dying`, or `Dead`.
- `float healthFast` / `float healthSlow`: Manages health values. `MaxHealth` determines the cap (usually 100).
- `float maxStamina`, `float staminaFast`, `float staminaSlow`: Variables for stamina management.
- `PlayerMovement movement`, `PlayerArms arms`, `PlayerCamera cam`, `PlayerInventory inventory`: Components for control, rendering, and items.
- Uses `IDamageTarget` and `IDamageTaker` interfaces for damage events.

### Zombie Structure
Zombies are represented by the `Zombie` class.
- `ZombieState state`: Enum tracking state like `Unaware`, `Aware`, `Attack`, `Jump`, `Dying`, etc.
- `ZombieIdentity identity`: Contains basic type info, including whether it's a boss (`IsBoss`).
- `ZombieHealth health`: Tracks health (amount/max). Top health scaling is based on `TopHealth` property or `BossfightController` overrides.
- `ZombieTargeting Targeting`: Component for player aggro tracking.

### Overall Architecture
- Implements a server-client multiplayer model using standard synchronization techniques (`PlayerPositionSynchronizer`, `ZombieNetSyncs`, `ClientSpeaker`).
- The game state is largely orchestrated by controller singletons (e.g., `LobbyController`, `PlayersController`, `ZombieController`, `MatchController`).

## Project Map

### Core System (`src/main.cpp`)
Initializes hooks and sets up the environment when injected.

### User Interface (`src/modules/ui/`)
- `Menu.hpp` / `Menu.cpp`: Handles dual DX11/DX12 hook-based initialization using MinHook and ImGui.
- **Pattern**: Centralized `ConfigData` struct holds all feature toggles. Call `UI::Initialize()` to bootstrap the menu overlay.

### Utilities (`src/utils/`)
- `mono_resolver.hpp` / `.cpp`: Resolves Mono framework functions dynamically from `mono-2.0-bdwgc.dll`.
- **Pattern**: Use `Utils::Mono::Resolve()` to interact with managed C# objects without hardcoding offsets.

### Features (`src/features/`)
*(Currently setting up the framework)*. Features will be grouped by domain (`Player/`, `Combat/`) mapping to the ImGui tabs.
