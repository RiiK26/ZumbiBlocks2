#include "player.hpp"
#include "../../modules/hooks/hook.hpp"
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"

typedef void (*TakeDamage_t)(void* instance, void* damage);
TakeDamage_t oTakeDamage = nullptr;

void hkTakeDamage(void* instance, void* damage)
{
  Logger::Log("[Invincible] hkTakeDamage called!");

  if (UI::Config.bInvincible) {
    Logger::Log("[Invincible] Blocked damage because bInvincible is true.");
    // Ignore damage
    return;
  }

  if (oTakeDamage) {
    oTakeDamage(instance, damage);
  }
}

namespace Features
{
  namespace Player
  {
    void Initialize()
    {
      Logger::Log("[Invincible] Attempting to hook TakeDamage...");
      void* target = Mono::CompileMethod("", "PlayerMain", "TakeDamage", 1);
      if (target) {
        Logger::Log("[Invincible] TakeDamage compiled to %p", target);
        int status = MH_CreateHook(target, (void*) hkTakeDamage, (void**) &oTakeDamage);
        if (status == MH_OK) {
          MH_EnableHook(target);
          Logger::Log("[Invincible] Successfully hooked TakeDamage!");
        }
        else {
          Logger::Log("[Invincible] MH_CreateHook failed with status %d", status);
        }
      }
      else {
        Logger::Log("[Invincible] Failed to compile PlayerMain::TakeDamage!");
      }
    }

    void Uninitialize()
    {
      // Hooks are globally disabled in Hooks::Uninitialize
    }
  }  // namespace Player
}  // namespace Features
