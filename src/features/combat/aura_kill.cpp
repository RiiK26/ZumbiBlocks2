#include "combat.hpp"
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/mono_resolver.hpp"

#include <cmath>    // IWYU pragma: keep
#include <cstdint>  // IWYU pragma: keep
#include <vector>
#include <windows.h>

namespace Features
{
  namespace Combat
  {
    static Mono::MonoField*  g_ZombieLoaderInstance           = nullptr;
    static Mono::MonoField*  g_ZombieLoaderZombies            = nullptr;
    static Mono::MonoField*  g_PlayerEntityLocField           = nullptr;
    static Mono::MonoField*  g_ZombieEntityLocField           = nullptr;
    static Mono::MonoField*  g_EntityPos3DField               = nullptr;
    static Mono::MonoField*  g_ZombieHealthField              = nullptr;
    static Mono::MonoField*  g_HealthAmountField              = nullptr;
    static Mono::MonoField*  g_HealthIsAliveField             = nullptr;
    static Mono::MonoField*  g_ZombieStateField               = nullptr;
    static Mono::MonoField*  g_ZombieTargetStateField         = nullptr;
    static Mono::MonoField*  g_ZombieIdentityField            = nullptr;

    static Mono::MonoClass*  g_DamageClass                    = nullptr;
    static Mono::MonoMethod* g_DamageCtor                     = nullptr;
    static Mono::MonoField*  g_DamageZombieDmgField           = nullptr;
    static Mono::MonoField*  g_DamageTypeField                = nullptr;
    static Mono::MonoClass*  g_ZombieClass                    = nullptr;
    static Mono::MonoMethod* g_TakeDamageMethod               = nullptr;

    static Mono::MonoField*  g_ZombieCtrlInstanceField        = nullptr;
    static Mono::MonoMethod* g_OnKilledZombiePersonallyMethod = nullptr;
    static Mono::MonoField*  g_PersonalKillsField             = nullptr;
    static Mono::MonoMethod* g_GetPersonalKillsMethod         = nullptr;
    static Mono::MonoMethod* g_AddPersonalKillMethod          = nullptr;

    struct Vector3
    {
      float x, y, z;
    };

    struct ZombieDamageNative
    {
      void* sourcePlayer;
      void* stagger;
      int   limb;
    };

    struct ZombieIdentityNative
    {
      int     id;
      int     type;
      uint8_t meshIndex;
      uint8_t materialIndex;
    };

    static int GetPersonalKills()
    {
      if (g_GetPersonalKillsMethod) {
        Mono::MonoObject* exc = nullptr;
        Mono::MonoObject* res =
          (Mono::MonoObject*) Mono::RuntimeInvoke(g_GetPersonalKillsMethod, nullptr, nullptr, &exc);
        if (res && !exc && Mono::mono_object_unbox) {
          return *(int*) Mono::mono_object_unbox(res);
        }
      }
      if (g_PersonalKillsField) {
        int val = 0;
        Mono::GetStaticFieldValue(g_PersonalKillsField, &val);
        return val;
      }
      return -1;
    }

    void InitializeAuraKill()
    {
      g_ZombieLoaderInstance   = Mono::GetFieldFromName("", "ZombieLoader", "Instance");
      g_ZombieLoaderZombies    = Mono::GetFieldFromName("", "ZombieLoader", "zombies");

      g_PlayerEntityLocField   = Mono::GetFieldFromName("", "PlayerMain", "entityLocation");
      g_ZombieEntityLocField   = Mono::GetFieldFromName("", "Zombie", "entityLocation");
      g_EntityPos3DField       = Mono::GetFieldFromName("", "EntityLocation", "pos3d");

      g_ZombieHealthField      = Mono::GetFieldFromName("", "Zombie", "health");
      g_HealthAmountField      = Mono::GetFieldFromName("", "ZombieHealth", "amount");
      g_HealthIsAliveField     = Mono::GetFieldFromName("", "ZombieHealth", "isAlive");
      g_ZombieStateField       = Mono::GetFieldFromName("", "Zombie", "state");
      g_ZombieTargetStateField = Mono::GetFieldFromName("", "Zombie", "targetState");
      g_ZombieIdentityField    = Mono::GetFieldFromName("", "Zombie", "identity");

      g_DamageClass            = Mono::GetClassFromName("", "Damage");
      if (g_DamageClass && Mono::mono_class_get_method_from_name) {
        g_DamageCtor           = Mono::mono_class_get_method_from_name(g_DamageClass, ".ctor", 1);
        g_DamageZombieDmgField = Mono::GetFieldFromName("", "Damage", "zombieDamage");
        g_DamageTypeField      = Mono::GetFieldFromName("", "Damage", "damageType");
      }

      g_ZombieClass = Mono::GetClassFromName("", "Zombie");
      if (g_ZombieClass && Mono::mono_class_get_method_from_name) {
        g_TakeDamageMethod = Mono::mono_class_get_method_from_name(g_ZombieClass, "TakeDamage", 1);
      }

      g_ZombieCtrlInstanceField        = Mono::GetFieldFromName("", "ZombieController", "instance");
      g_OnKilledZombiePersonallyMethod = Mono::GetMethodFromName("", "ZombieController", "OnKilledZombiePersonally", 1);
      g_PersonalKillsField = Mono::GetFieldFromName("", "MatchStatsTracking", "<PersonalZombieKills>k__BackingField");
      g_GetPersonalKillsMethod = Mono::GetMethodFromName("", "MatchStatsTracking", "get_PersonalZombieKills", 0);
      g_AddPersonalKillMethod  = Mono::GetMethodFromName("", "MatchStatsTracking", "AddPersonalZombieKill", 0);
    }

    void UpdateAuraKill(void* playerInstance)
    {
      if (!playerInstance)
        return;

      if (!g_PlayerEntityLocField || !g_EntityPos3DField) {
        InitializeAuraKill();
        if (!g_PlayerEntityLocField || !g_EntityPos3DField)
          return;
      }

      void* playerEntityLocObj = nullptr;
      Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerEntityLocField, &playerEntityLocObj);
      if (!playerEntityLocObj)
        return;

      Vector3 playerPos = {0.0f, 0.0f, 0.0f};
      Mono::GetFieldValue((Mono::MonoObject*) playerEntityLocObj, g_EntityPos3DField, &playerPos);

      // Locate ZombieLoader instance via static Instance field
      void* loaderInstance = nullptr;
      if (g_ZombieLoaderInstance) {
        Mono::GetStaticFieldValue(g_ZombieLoaderInstance, &loaderInstance);
      }
      if (!loaderInstance)
        return;

      void* zombiesList = nullptr;
      if (g_ZombieLoaderZombies) {
        Mono::GetFieldValue((Mono::MonoObject*) loaderInstance, g_ZombieLoaderZombies, &zombiesList);
      }
      if (!zombiesList)
        return;

      std::vector<void*> zombies;
      if (!Mono::GetListElements(zombiesList, zombies) || zombies.empty())
        return;

      const float       maxDistSq = 15.0f * 15.0f;  // 15 meters aura radius
      Mono::MonoDomain* domain    = Mono::GetActiveDomain();

      for (void* z : zombies) {
        if (!z)
          continue;

        int state = 0;
        if (g_ZombieStateField) {
          Mono::GetFieldValue((Mono::MonoObject*) z, g_ZombieStateField, &state);
        }
        int targetState = 0;
        if (g_ZombieTargetStateField) {
          Mono::GetFieldValue((Mono::MonoObject*) z, g_ZombieTargetStateField, &targetState);
        }

        // State 7 = Dying, State 8 = Dead. If already dying or dead, do not touch.
        if (state == 7 || state == 8 || targetState == 7 || targetState == 8)
          continue;

        void* zombieEntityLocObj = nullptr;
        if (g_ZombieEntityLocField) {
          Mono::GetFieldValue((Mono::MonoObject*) z, g_ZombieEntityLocField, &zombieEntityLocObj);
        }
        if (!zombieEntityLocObj)
          continue;

        Vector3 zPos = {0.0f, 0.0f, 0.0f};
        Mono::GetFieldValue((Mono::MonoObject*) zombieEntityLocObj, g_EntityPos3DField, &zPos);

        float dx        = playerPos.x - zPos.x;
        float dy        = playerPos.y - zPos.y;
        float dz        = playerPos.z - zPos.z;
        float distSq    = (dx * dx) + (dy * dy) + (dz * dz);

        void* healthObj = nullptr;
        if (g_ZombieHealthField) {
          Mono::GetFieldValue((Mono::MonoObject*) z, g_ZombieHealthField, &healthObj);
        }

        if (UI::Config.bAuraKill && distSq <= maxDistSq) {
          // Zombie is inside AuraKill radius: KILL CLEANLY via TakeDamage
          if (healthObj && g_HealthIsAliveField && g_HealthAmountField) {
            bool  curAlive  = true;
            float curAmount = 100.0f;
            Mono::GetFieldValue((Mono::MonoObject*) healthObj, g_HealthIsAliveField, &curAlive);
            Mono::GetFieldValue((Mono::MonoObject*) healthObj, g_HealthAmountField, &curAmount);
            if (!curAlive || curAmount <= 0.0f) {
              bool  aliveVal = true;
              float hpVal    = 100.0f;
              Mono::SetFieldValue((Mono::MonoObject*) healthObj, g_HealthIsAliveField, &aliveVal);
              Mono::SetFieldValue((Mono::MonoObject*) healthObj, g_HealthAmountField, &hpVal);
            }
          }

          int zombieType = 0;
          if (g_ZombieIdentityField) {
            ZombieIdentityNative ident = {};
            Mono::GetFieldValue((Mono::MonoObject*) z, g_ZombieIdentityField, &ident);
            zombieType = ident.type;
          }

          int killsBefore = GetPersonalKills();

          if (
            g_DamageClass && g_DamageCtor && g_TakeDamageMethod && Mono::mono_object_new && Mono::mono_runtime_invoke
            && domain
          ) {
            Mono::MonoObject* dmgObj = Mono::mono_object_new(domain, g_DamageClass);
            if (dmgObj) {
              float             hugeDmg     = 99999.0f;
              void*             ctorArgs[1] = {&hugeDmg};
              Mono::MonoObject* exc         = nullptr;
              Mono::mono_runtime_invoke(g_DamageCtor, dmgObj, ctorArgs, &exc);

              if (g_DamageTypeField) {
                int dmgType = 3;  // DamageType.Shot
                Mono::SetFieldValue(dmgObj, g_DamageTypeField, &dmgType);
              }

              if (g_DamageZombieDmgField) {
                ZombieDamageNative zd = {};
                zd.sourcePlayer       = playerInstance;
                zd.stagger            = nullptr;
                zd.limb               = 0;
                Mono::SetFieldValue(dmgObj, g_DamageZombieDmgField, &zd);
              }

              void* tdArgs[1] = {dmgObj};
              Mono::mono_runtime_invoke(g_TakeDamageMethod, z, tdArgs, &exc);
            }
          }

          int killsAfter = GetPersonalKills();

          // If kill count did not increment automatically, explicitly register personal kill
          if (killsAfter == killsBefore || killsBefore == -1) {
            void* zombieCtrlInst = nullptr;
            if (g_ZombieCtrlInstanceField) {
              Mono::GetStaticFieldValue(g_ZombieCtrlInstanceField, &zombieCtrlInst);
            }
            if (zombieCtrlInst && g_OnKilledZombiePersonallyMethod) {
              void* killArgs[1] = {&zombieType};
              Mono::RuntimeInvoke(g_OnKilledZombiePersonallyMethod, zombieCtrlInst, killArgs);
            }
            else if (g_AddPersonalKillMethod) {
              Mono::RuntimeInvoke(g_AddPersonalKillMethod, nullptr);
            }
          }
        }
        else {
          // Zombie is outside aura (or AuraKill is off):
          // Recover any previously glitched zombie (isAlive == false or amount <= 0 while not dead)
          // so bullets and regular combat can damage and kill it properly!
          if (healthObj && g_HealthIsAliveField && g_HealthAmountField) {
            bool  curIsAlive = true;
            float curAmount  = 100.0f;
            Mono::GetFieldValue((Mono::MonoObject*) healthObj, g_HealthIsAliveField, &curIsAlive);
            Mono::GetFieldValue((Mono::MonoObject*) healthObj, g_HealthAmountField, &curAmount);
            if (!curIsAlive || curAmount <= 0.0f) {
              bool  fixAlive = true;
              float normalHp = 100.0f;
              Mono::SetFieldValue((Mono::MonoObject*) healthObj, g_HealthIsAliveField, &fixAlive);
              Mono::SetFieldValue((Mono::MonoObject*) healthObj, g_HealthAmountField, &normalHp);
            }
          }
        }
      }
    }
  }  // namespace Combat
}  // namespace Features
