#include "combat.hpp"
#include "../../modules/hooks/hook.hpp"    // IWYU pragma: keep
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"  // IWYU pragma: keep
#include "../../modules/utils/mono_resolver.hpp"

namespace Features
{
  namespace Combat
  {
    void InitializeAmmo() { }

    void UpdateAmmo(void* playerInstance)
    {
      if (!UI::Config.bInfiniteAmmo || !playerInstance)
        return;

      // 1. Set ammoConsumption = 0 on equipped gun so firing uses 0 ammo
      static Mono::MonoField* armsField     = nullptr;
      static Mono::MonoField* handsField    = nullptr;
      static Mono::MonoField* gunField      = nullptr;
      static Mono::MonoField* dbGunField    = nullptr;
      static Mono::MonoField* ammoConsField = nullptr;

      if (!armsField) {
        armsField     = Mono::GetFieldFromName("", "PlayerMain", "arms");
        handsField    = Mono::GetFieldFromName("", "PlayerArms", "hands");
        gunField      = Mono::GetFieldFromName("", "PlayerHandItems", "<Gun>k__BackingField");
        dbGunField    = Mono::GetFieldFromName("", "PhysicalGun", "<DbReference>k__BackingField");
        ammoConsField = Mono::GetFieldFromName("", "DatabaseGun", "ammoConsumption");
      }

      if (armsField && handsField && gunField && dbGunField && ammoConsField) {
        void* armsObj = nullptr;
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, armsField, &armsObj);
        if (armsObj) {
          void* handsObj = nullptr;
          Mono::GetFieldValue((Mono::MonoObject*) armsObj, handsField, &handsObj);
          if (handsObj) {
            void* gunObj = nullptr;
            Mono::GetFieldValue((Mono::MonoObject*) handsObj, gunField, &gunObj);
            if (gunObj) {
              void* dbGunObj = nullptr;
              Mono::GetFieldValue((Mono::MonoObject*) gunObj, dbGunField, &dbGunObj);
              if (dbGunObj) {
                int zeroConsumption = 0;
                Mono::SetFieldValue((Mono::MonoObject*) dbGunObj, ammoConsField, &zeroConsumption);
              }
            }
          }
        }
      }

      // 2. Refill equipped weapons magazine to 999
      static Mono::MonoField* invField          = nullptr;
      static Mono::MonoField* eqField           = nullptr;
      static Mono::MonoField* wepField          = nullptr;
      static Mono::MonoField* ammoField         = nullptr;
      static Mono::MonoField* stackField        = nullptr;
      static Mono::MonoField* storageField      = nullptr;
      static Mono::MonoField* storageItemsField = nullptr;

      if (!invField) {
        invField          = Mono::GetFieldFromName("", "PlayerMain", "inventory");
        eqField           = Mono::GetFieldFromName("", "PlayerInventory", "equippedItems");
        wepField          = Mono::GetFieldFromName("", "PlayerEquippedItems", "weapons");
        ammoField         = Mono::GetFieldFromName("", "InventoryItem", "ammo");
        stackField        = Mono::GetFieldFromName("", "InventoryItem", "stackCount");
        storageField      = Mono::GetFieldFromName("", "PlayerInventory", "storage");
        storageItemsField = Mono::GetFieldFromName("", "ItemContainer", "items");
      }

      if (!invField || !eqField || !wepField || !ammoField)
        return;

      void* invObj = nullptr;
      Mono::GetFieldValue((Mono::MonoObject*) playerInstance, invField, &invObj);
      if (!invObj)
        return;

      void* eqObj = nullptr;
      Mono::GetFieldValue((Mono::MonoObject*) invObj, eqField, &eqObj);
      if (eqObj) {
        void* wepList = nullptr;
        Mono::GetFieldValue((Mono::MonoObject*) eqObj, wepField, &wepList);
        if (wepList) {
          std::vector<void*> weaponItems;
          if (Mono::GetListElements(wepList, weaponItems)) {
            int maxAmmo = 999;
            for (void* item : weaponItems) {
              if (item) {
                Mono::SetFieldValue((Mono::MonoObject*) item, ammoField, &maxAmmo);
              }
            }
          }
        }
      }

      // 3. Refill backpack/storage reserve stacks to 999
      if (storageField && storageItemsField && stackField) {
        void* storageObj = nullptr;
        Mono::GetFieldValue((Mono::MonoObject*) invObj, storageField, &storageObj);
        if (storageObj) {
          void* storageItemsList = nullptr;
          Mono::GetFieldValue((Mono::MonoObject*) storageObj, storageItemsField, &storageItemsList);
          if (storageItemsList) {
            std::vector<void*> storageItems;
            if (Mono::GetListElements(storageItemsList, storageItems)) {
              int maxStack = 999;
              for (void* item : storageItems) {
                if (item) {
                  Mono::SetFieldValue((Mono::MonoObject*) item, stackField, &maxStack);
                }
              }
            }
          }
        }
      }
    }
  }  // namespace Combat
}  // namespace Features
