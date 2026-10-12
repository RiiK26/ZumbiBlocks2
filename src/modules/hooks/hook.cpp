#include "hook.hpp"

#include "../../features/player/player.hpp"
#include "../../features/combat/combat.hpp"

#include <MinHook.h>

bool Hooks::bHooksFailed = false;

void Hooks::Initialize()
{
  int mhStatus = MH_Initialize();
  if (mhStatus != MH_OK && mhStatus != MH_ERROR_ALREADY_INITIALIZED) {
    Hooks::bHooksFailed = true;
  }

  Features::Player::InitializeInvicible();
  Features::Player::InitializeStamina();
  Features::Combat::Initialize();
  Features::Combat::InitializeAmmo();
  Features::Combat::InitializeAuraKill();
}

void Hooks::Uninitialize()
{
  Features::Combat::Uninitialize();
  MH_DisableHook(MH_ALL_HOOKS);
  MH_Uninitialize();
}
