#include "combat.hpp"
#include "../../modules/hooks/hook.hpp"  // IWYU pragma: keep
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"
#include "../../modules/utils/mono_resolver.hpp"

typedef void (*ZombieTakeDamage_t)(void* instance, void* damage);
ZombieTakeDamage_t oZombieTakeDamage = nullptr;

Mono::MonoField*   damageAmountField = nullptr;

void hkZombieTakeDamage(void* instance, void* damage)
{
  static bool logged = false;
  if (!logged) {
    Logger::Log("[OneHitKill] hkZombieTakeDamage called!");
    logged = true;
  }

  if (UI::Config.bOneHitKill && damage && damageAmountField) {
    float hugeDamage = 999999999.0f;
    Mono::SetFieldValue((Mono::MonoObject*) damage, damageAmountField, &hugeDamage);
  }

  if (oZombieTakeDamage) {
    oZombieTakeDamage(instance, damage);
  }
}

namespace Features
{
  namespace Combat
  {
    void Initialize()
    {
      Logger::Log("[Combat] Initializing One Hit Kill...");

      void* targetUpdate = Mono::CompileMethod("", "Zombie", "TakeDamage", 1);
      if (targetUpdate) {
        if (MH_CreateHook(targetUpdate, (void*) hkZombieTakeDamage, (void**) &oZombieTakeDamage) == MH_OK) {
          MH_EnableHook(targetUpdate);
          Logger::Log("[Combat] Successfully hooked Zombie::TakeDamage!");
        }
        else {
          Logger::Log("[Combat] Failed to hook Zombie::TakeDamage!");
        }
      }
      else {
        Logger::Log("[Combat] Failed to compile Zombie::TakeDamage!");
      }

      damageAmountField = Mono::GetFieldFromName("", "Damage", "amount");
      if (damageAmountField) {
        Logger::Log("[Combat] Successfully resolved Damage::amount field!");
      }
      else {
        Logger::Log("[Combat] Failed to resolve Damage::amount field!");
      }
    }

    void Uninitialize()
    {
      // Hooks are uninitialized globally
    }
  }  // namespace Combat
}  // namespace Features
