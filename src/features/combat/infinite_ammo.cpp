#include "combat.hpp"
#include "../../modules/hooks/hook.hpp"  // IWYU pragma: keep
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"
#include "../../modules/utils/mono_resolver.hpp"

typedef void (*ShootGun_t)(void* instance, void* equippedGunItem);
ShootGun_t       oShootGun              = nullptr;

Mono::MonoField* inventoryItemAmmoField = nullptr;

void hkShootGun(void* instance, void* equippedGunItem)
{
  if (UI::Config.bInfiniteAmmo && equippedGunItem && inventoryItemAmmoField) {
    int ammo = 0;
    Mono::GetFieldValue((Mono::MonoObject*) equippedGunItem, inventoryItemAmmoField, &ammo);

    if (oShootGun) {
      oShootGun(instance, equippedGunItem);
    }

    // Restore ammo so it's not consumed
    Mono::SetFieldValue((Mono::MonoObject*) equippedGunItem, inventoryItemAmmoField, &ammo);
  }
  else {
    if (oShootGun) {
      oShootGun(instance, equippedGunItem);
    }
  }
}

namespace Features
{
  namespace Combat
  {
    void InitializeAmmo()
    {
      Logger::Log("[Combat] Initializing Infinite Ammo...");

      void* targetShootGun = Mono::CompileMethod("", "PlayerArms", "ShootGun", 1);
      if (targetShootGun) {
        if (MH_CreateHook(targetShootGun, (void*) hkShootGun, (void**) &oShootGun) == MH_OK) {
          MH_EnableHook(targetShootGun);
          Logger::Log("[Combat] Successfully hooked PlayerArms::ShootGun!");
        }
        else {
          Logger::Log("[Combat] Failed to hook PlayerArms::ShootGun!");
        }
      }
      else {
        Logger::Log("[Combat] Failed to compile PlayerArms::ShootGun!");
      }

      inventoryItemAmmoField = Mono::GetFieldFromName("", "InventoryItem", "ammo");
      if (inventoryItemAmmoField) {
        Logger::Log("[Combat] Successfully resolved InventoryItem::ammo field!");
      }
      else {
        Logger::Log("[Combat] Failed to resolve InventoryItem::ammo field!");
      }
    }
  }  // namespace Combat
}  // namespace Features
