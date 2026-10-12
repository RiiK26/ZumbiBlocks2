#pragma once
#include <string>

namespace UI
{
  struct ConfigData
  {
    bool bMenuOpen        = true;
    bool bInvincible      = false;
    bool bInfiniteStamina = false;
    bool bOneHitKill      = false;
    bool bInfiniteAmmo    = false;
    bool bEnableCrosshair = false;
    int  iCrosshairType   = 0;
    bool bAuraKill        = false;
    // Add specific settings below here

    void LoadConfig();
    void SaveConfig();
  };

  // Global config instance
  extern ConfigData Config;

  std::string GetModuleDirectory();
  std::string GetConfigPath();
}  // namespace UI
