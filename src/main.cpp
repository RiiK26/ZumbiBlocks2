#include <windows.h>
#include "modules/utils/mono_resolver.hpp"
#include "modules/ui/hooks.hpp"
#include "modules/hooks/hook.hpp"
#include "modules/utils/logger.hpp"

DWORD WINAPI MainThread(LPVOID lpReserved)
{
  Logger::Initialize();
  Logger::Log("[MainThread] Started. Waiting for mono-2.0-bdwgc.dll...");

  // Wait for the Mono module to be loaded by the game before attempting to initialize
  while (!GetModuleHandleA("mono-2.0-bdwgc.dll")) {
    Sleep(100);
  }

  Logger::Log("[MainThread] mono-2.0-bdwgc.dll found. Initializing Mono...");

  if (!Mono::Init()) {
    Logger::Log("[MainThread] ERROR: Mono::Init() failed!");
    FreeLibraryAndExitThread((HMODULE) lpReserved, 0);
    return 0;
  }

  Logger::Log("[MainThread] Initializing Hooks...");
  // Initialize MinHook and our gameplay hooks
  Hooks::Initialize();
  Logger::Log("[MainThread] Hooks Initialized.");

  Logger::Log("[MainThread] Initializing UI...");
  // Initialize UI (DX11/DX12 hooks and ImGui)
  UI::Initialize();
  Logger::Log("[MainThread] UI Initialized.");

  Logger::Log("[MainThread] Entering wait loop (Press END to unload).");

  // Listen for an unload key (END)
  while (!(GetAsyncKeyState(VK_END) & 1)) {
    Sleep(100);
  }

  Logger::Log("[MainThread] END key pressed, unloading...");

  // Unload hooks and UI
  Hooks::Uninitialize();
  UI::Shutdown();

  Logger::Log("[MainThread] Exiting thread.");
  FreeLibraryAndExitThread((HMODULE) lpReserved, 0);
  return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH :
    DisableThreadLibraryCalls(hinstDLL);
    CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);
    break;
  case DLL_PROCESS_DETACH :
    break;
  }
  return TRUE;
}
