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

    // Movement Speed Editor
    void InitializeMovementSpeed();
    void UpdateMovementSpeed(void* playerInstance);
    void RestoreMovementSpeed(void* playerInstance);

    // No Clip
    void InitializeNoClip();
    void UpdateNoClip(void* playerInstance);
    void RestoreNoClip(void* playerInstance);
  }  // namespace Player
}  // namespace Features
