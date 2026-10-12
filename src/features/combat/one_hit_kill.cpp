#include "combat.hpp"
#include "../../modules/hooks/hook.hpp"           // IWYU pragma: keep
#include "../../modules/ui/config.hpp"            // IWYU pragma: keep
#include "../../modules/utils/logger.hpp"         // IWYU pragma: keep
#include "../../modules/utils/mono_resolver.hpp"  // IWYU pragma: keep

#include <windows.h>

namespace Features
{
  namespace Combat
  {
    void Initialize() { }

    void UpdateOneHitKill(void* playerInstance)
    {
      if (!UI::Config.bOneHitKill || !playerInstance)
        return;

      // 1. Boost equipped gun damage to 99999
      if (UI::Config.bOneHitKill) {
        static Mono::MonoField* armsField  = nullptr;
        static Mono::MonoField* handsField = nullptr;
        static Mono::MonoField* gunField   = nullptr;
        static Mono::MonoField* dbGunField = nullptr;
        static Mono::MonoField* dmgField   = nullptr;
        static Mono::MonoField* hsField    = nullptr;

        if (!armsField) {
          armsField  = Mono::GetFieldFromName("", "PlayerMain", "arms");
          handsField = Mono::GetFieldFromName("", "PlayerArms", "hands");
          gunField   = Mono::GetFieldFromName("", "PlayerHandItems", "<Gun>k__BackingField");
          dbGunField = Mono::GetFieldFromName("", "PhysicalGun", "<DbReference>k__BackingField");
          dmgField   = Mono::GetFieldFromName("", "DatabaseGun", "dmg");
          hsField    = Mono::GetFieldFromName("", "DatabaseGun", "hsMultiplier");
        }

        if (armsField && handsField && gunField && dbGunField && dmgField) {
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
                  float maxDmg = 99999.0f;
                  Mono::SetFieldValue((Mono::MonoObject*) dbGunObj, dmgField, &maxDmg);
                  if (hsField) {
                    float maxHs = 10.0f;
                    Mono::SetFieldValue((Mono::MonoObject*) dbGunObj, hsField, &maxHs);
                  }
                }
              }
            }
          }
        }
      }

      // 2. Heal and recover any glitched immortal zombies from previous bugs
      static Mono::MonoField* zombieLoaderInstField = nullptr;
      static Mono::MonoField* zombieLoaderListField = nullptr;
      static Mono::MonoField* zombieHealthField     = nullptr;
      static Mono::MonoField* healthAmountField     = nullptr;
      static Mono::MonoField* healthIsAliveField    = nullptr;
      static Mono::MonoField* zombieStateField      = nullptr;

      if (!zombieLoaderInstField) {
        zombieLoaderInstField = Mono::GetFieldFromName("", "ZombieLoader", "Instance");
        zombieLoaderListField = Mono::GetFieldFromName("", "ZombieLoader", "zombies");
        zombieHealthField     = Mono::GetFieldFromName("", "Zombie", "health");
        healthAmountField     = Mono::GetFieldFromName("", "ZombieHealth", "amount");
        healthIsAliveField    = Mono::GetFieldFromName("", "ZombieHealth", "isAlive");
        zombieStateField      = Mono::GetFieldFromName("", "Zombie", "state");
      }

      if (!zombieLoaderListField || !zombieHealthField || !healthAmountField || !healthIsAliveField)
        return;

      void* loaderInstance = nullptr;
      if (zombieLoaderInstField) {
        Mono::GetStaticFieldValue(zombieLoaderInstField, &loaderInstance);
      }
      if (!loaderInstance) {
        loaderInstance = Mono::FindObjectOfType("", "ZombieLoader");
      }
      if (!loaderInstance)
        return;

      void* zombiesList = nullptr;
      Mono::GetFieldValue((Mono::MonoObject*) loaderInstance, zombieLoaderListField, &zombiesList);
      if (!zombiesList)
        return;

      std::vector<void*> zombies;
      if (Mono::GetListElements(zombiesList, zombies)) {
        for (void* z : zombies) {
          if (!z)
            continue;

          int state = 0;
          if (zombieStateField) {
            Mono::GetFieldValue((Mono::MonoObject*) z, zombieStateField, &state);
          }

          // If a zombie is alive (not Dying=7, not Dead=8), but its health was glitched (isAlive == false or amount <= 0),
          // restore it so bullets and weapons can kill it properly!
          if (state != 7 && state != 8) {
            void* healthObj = nullptr;
            Mono::GetFieldValue((Mono::MonoObject*) z, zombieHealthField, &healthObj);
            if (healthObj) {
              bool  curIsAlive = true;
              float curAmount  = 100.0f;
              Mono::GetFieldValue((Mono::MonoObject*) healthObj, healthIsAliveField, &curIsAlive);
              Mono::GetFieldValue((Mono::MonoObject*) healthObj, healthAmountField, &curAmount);
              if (!curIsAlive || curAmount <= 0.0f) {
                bool  fixAlive = true;
                float normalHp = 100.0f;
                Mono::SetFieldValue((Mono::MonoObject*) healthObj, healthIsAliveField, &fixAlive);
                Mono::SetFieldValue((Mono::MonoObject*) healthObj, healthAmountField, &normalHp);
              }
            }
          }
        }
      }
    }

    void Uninitialize() { }
  }  // namespace Combat
}  // namespace Features
