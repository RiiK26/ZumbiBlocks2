#include "player.hpp"
#include "../../modules/hooks/hook.hpp"  // IWYU pragma: keep
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"

typedef void (*TakeDamage_t)(void* instance, void* damage);
TakeDamage_t oTakeDamage = nullptr;

void hkTakeDamage(void* instance, void* damage)
{
  static bool logged = false;
  if (!logged) {
    Logger::Log("[Invincible] hkTakeDamage called!");
    logged = true;
  }

  if (UI::Config.bInvincible) {
    return;
  }

  if (oTakeDamage) {
    oTakeDamage(instance, damage);
  }
}
typedef void (*DirectDamage_t)(void* instance, void* damage);
DirectDamage_t oDirectDamage = nullptr;

void hkDirectDamage(void* instance, void* damage)
{
  static bool logged = false;
  if (!logged) {
    Logger::Log("[Invincible] hkDirectDamage called!");
    logged = true;
  }
  if (UI::Config.bInvincible) {
    return;
  }
  if (oDirectDamage) {
    oDirectDamage(instance, damage);
  }
}

namespace Features
{
  namespace Player
  {
    void InitializeInvicible()
    {
      Logger::Log("[Invincible] Attempting to hook TakeDamage and DirectDamage...");
      void* targetTakeDamage = Mono::CompileMethod("", "PlayerMain", "TakeDamage", 1);
      if (targetTakeDamage) {
        if (MH_CreateHook(targetTakeDamage, (void*) hkTakeDamage, (void**) &oTakeDamage) == MH_OK) {
          MH_EnableHook(targetTakeDamage);
          Logger::Log("[Invincible] Successfully hooked TakeDamage!");
        }
      }

      void* targetDirectDamage = Mono::CompileMethod("", "PlayerMain", "DirectDamage", 1);
      if (targetDirectDamage) {
        if (MH_CreateHook(targetDirectDamage, (void*) hkDirectDamage, (void**) &oDirectDamage) == MH_OK) {
          MH_EnableHook(targetDirectDamage);
          Logger::Log("[Invincible] Successfully hooked DirectDamage!");
        }
      }
    }

    void UninitializeInvicible()
    {
      // Hooks are globally disabled in Hooks::Uninitialize
    }
  }  // namespace Player
}  // namespace Features
