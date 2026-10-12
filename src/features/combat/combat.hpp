#pragma once

namespace Features
{
  namespace Combat
  {
    void Initialize();
    void InitializeAmmo();
    void InitializeAuraKill();
    void UpdateAuraKill(void* playerInstance);
    void UpdateAmmo(void* playerInstance);
    void UpdateOneHitKill(void* playerInstance);
    void Uninitialize();
  }  // namespace Combat
}  // namespace Features
