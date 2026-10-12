#include "player.hpp"
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"
#include "../../modules/utils/mono_resolver.hpp"
#include <string>

namespace Features
{
  namespace Player
  {
    static std::string s_unlockStatus = "";

    const char* GetUnlockStatus()
    {
      return s_unlockStatus.c_str();
    }

    bool UnlockAllLoadouts()
    {
      Mono::EnsureThreadAttached();

      // 1. Bypass demo / free pricing restrictions by setting ZBVersion.Instance.pricing = Paid (1)
      static Mono::MonoField* zbInstanceField = nullptr;
      static Mono::MonoField* zbPricingField  = nullptr;
      if (!zbPricingField) {
        zbInstanceField = Mono::GetFieldFromName("", "ZBVersion", "<Instance>k__BackingField");
        zbPricingField  = Mono::GetFieldFromName("", "ZBVersion", "pricing");
      }

      void* zbInstance = nullptr;
      if (zbInstanceField) {
        Mono::GetStaticFieldValue(zbInstanceField, &zbInstance);
      }
      if (zbInstance && zbPricingField) {
        int paidVal = 1;  // VersionPricing.Paid
        Mono::SetFieldValue((Mono::MonoObject*) zbInstance, zbPricingField, &paidVal);
        Logger::Log("[LoadoutUnlocker] Set ZBVersion pricing to Paid (1)");
      }

      // 2. Locate LoadoutSelector via static singleton or MatchController
      static Mono::MonoField*  loadoutSelInstanceField = nullptr;
      static Mono::MonoField*  unlockedLoadoutsField   = nullptr;
      static Mono::MonoMethod* unlockAllMethod         = nullptr;

      if (!unlockedLoadoutsField) {
        loadoutSelInstanceField = Mono::GetFieldFromName("", "LoadoutSelector", "instance");
        unlockedLoadoutsField   = Mono::GetFieldFromName("", "LoadoutSelector", "unlockedLoadouts");
        unlockAllMethod         = Mono::GetMethodFromName("", "LoadoutSelector", "UnlockAll", 0);
      }

      void* loadoutInstance = nullptr;
      if (loadoutSelInstanceField) {
        Mono::GetStaticFieldValue(loadoutSelInstanceField, &loadoutInstance);
      }

      if (!loadoutInstance) {
        // Fallback to MatchController.instance.loadoutSelector
        static Mono::MonoField* matchInstanceField = nullptr;
        static Mono::MonoField* matchLoadoutField  = nullptr;
        if (!matchLoadoutField) {
          matchInstanceField = Mono::GetFieldFromName("", "MatchController", "instance");
          matchLoadoutField  = Mono::GetFieldFromName("", "MatchController", "loadoutSelector");
        }
        void* matchInstance = nullptr;
        if (matchInstanceField) {
          Mono::GetStaticFieldValue(matchInstanceField, &matchInstance);
        }
        if (matchInstance && matchLoadoutField) {
          Mono::GetFieldValue((Mono::MonoObject*) matchInstance, matchLoadoutField, &loadoutInstance);
        }
      }

      if (!loadoutInstance) {
        s_unlockStatus = "Error: LoadoutSelector not ready yet";
        Logger::Log("[LoadoutUnlocker] LoadoutSelector instance not found");
        return false;
      }

      // Clear unlockedLoadouts list to prevent duplicate entries before repopulating
      if (unlockedLoadoutsField) {
        void* listObj = nullptr;
        Mono::GetFieldValue((Mono::MonoObject*) loadoutInstance, unlockedLoadoutsField, &listObj);
        if (listObj && Mono::mono_object_get_class && Mono::mono_class_get_method_from_name) {
          Mono::MonoClass* listClass = (Mono::MonoClass*) Mono::mono_object_get_class((Mono::MonoObject*) listObj);
          if (listClass) {
            Mono::MonoMethod* clearMethod = Mono::mono_class_get_method_from_name(listClass, "Clear", 0);
            if (clearMethod) {
              Mono::RuntimeInvoke(clearMethod, listObj);
            }
          }
        }
      }

      // Invoke UnlockAll() on LoadoutSelector
      if (unlockAllMethod) {
        Mono::RuntimeInvoke(unlockAllMethod, loadoutInstance);
        Logger::Log("[LoadoutUnlocker] Invoked LoadoutSelector.UnlockAll()");
      }

      // 3. Refresh Perks / Accessories menu
      static Mono::MonoField*  perkInstanceField = nullptr;
      static Mono::MonoMethod* perkRefreshMethod = nullptr;
      if (!perkRefreshMethod) {
        perkInstanceField = Mono::GetFieldFromName("", "PerkSelectionMenu", "instance");
        perkRefreshMethod = Mono::GetMethodFromName("", "PerkSelectionMenu", "RefreshAll", 0);
      }

      void* perkInstance = nullptr;
      if (perkInstanceField) {
        Mono::GetStaticFieldValue(perkInstanceField, &perkInstance);
      }
      if (perkInstance && perkRefreshMethod) {
        Mono::RuntimeInvoke(perkRefreshMethod, perkInstance);
        Logger::Log("[LoadoutUnlocker] Invoked PerkSelectionMenu.RefreshAll()");
      }

      // 4. Save progress file immediately to disk so unlocks persist
      static Mono::MonoField*  persistenceInstanceField = nullptr;
      static Mono::MonoMethod* saveFileMethod            = nullptr;
      if (!saveFileMethod) {
        persistenceInstanceField = Mono::GetFieldFromName("", "PersistenceController", "instance");
        saveFileMethod            = Mono::GetMethodFromName("", "PersistenceController", "SaveFile", 1);
      }

      void* persistenceInstance = nullptr;
      if (persistenceInstanceField) {
        Mono::GetStaticFieldValue(persistenceInstanceField, &persistenceInstance);
      }
      if (persistenceInstance && saveFileMethod) {
        int   progressFileId = 1;  // FileID.ProgressFile
        void* saveArgs[1]    = {&progressFileId};
        Mono::RuntimeInvoke(saveFileMethod, persistenceInstance, saveArgs);
        Logger::Log("[LoadoutUnlocker] Saved progress file to disk");
      }

      s_unlockStatus = "Unlocked: All weapons, skins & perks ready!";
      return true;
    }

    void UpdateLoadoutUnlocker()
    {
      if (!UI::Config.bUnlockAllLoadouts)
        return;

      static bool s_autoUnlocked = false;
      if (!s_autoUnlocked) {
        if (UnlockAllLoadouts()) {
          s_autoUnlocked = true;
        }
      }
    }
  }  // namespace Player
}  // namespace Features
