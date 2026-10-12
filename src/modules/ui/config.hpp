#pragma once
#include <string>

namespace UI
{
  struct ConfigData
  {
    bool  bMenuOpen                = true;
    bool  bInvincible              = false;
    bool  bInfiniteStamina         = false;
    bool  bOneHitKill              = false;
    bool  bInfiniteAmmo            = false;
    bool  bEnableCrosshair         = false;
    int   iCrosshairType           = 0;
    bool  bAuraKill                = false;
    bool  bUnlockAllLoadouts       = false;
    bool  bMovementSpeed           = false;
    float fMovementSpeedMultiplier = 2.0f;
    bool  bNoClip                  = false;
    float fNoClipSpeed             = 15.0f;
    // Add specific settings below here

    void LoadConfig();
    void SaveConfig();
  };

  // Global config instance
  extern ConfigData Config;

  std::string GetModuleDirectory();
  std::string GetConfigPath();
}  // namespace UI
