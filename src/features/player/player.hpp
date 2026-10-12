#pragma once

namespace Features
{
  namespace Player
  {
    void InitializeInvicible();
    void UpdateInvincible(void* playerInstance);
    void InitializeStamina();

    void UninitializeInvicible();

    // Loadout & Appearance Unlocker
    bool UnlockAllLoadouts();
    void UpdateLoadoutUnlocker();
    const char* GetUnlockStatus();
  }  // namespace Player
}  // namespace Features
