#include "player.hpp"
#include "../../modules/hooks/hook.hpp"  // IWYU pragma: keep
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"
#include "../../modules/utils/mono_resolver.hpp"

typedef void (*Update_t)(void* instance);
Update_t         oPlayerMainUpdate = nullptr;

Mono::MonoField* staminaFastField  = nullptr;
Mono::MonoField* staminaSlowField  = nullptr;
Mono::MonoField* maxStaminaField   = nullptr;

void hkPlayerMainUpdate(void* instance)
{
  if (oPlayerMainUpdate) {
    oPlayerMainUpdate(instance);
  }

  if (UI::Config.bInfiniteStamina && instance && maxStaminaField && staminaFastField && staminaSlowField) {
    float maxStamina = 0.0f;
    Mono::GetFieldValue((Mono::MonoObject*) instance, maxStaminaField, &maxStamina);

    if (maxStamina > 0.0f) {
      Mono::SetFieldValue((Mono::MonoObject*) instance, staminaFastField, &maxStamina);
      Mono::SetFieldValue((Mono::MonoObject*) instance, staminaSlowField, &maxStamina);
    }
  }
}

namespace Features
{
  namespace Player
  {
    void InitializeStamina()
    {
      Logger::Log("[InfiniteStamina] Initializing...");

      void* targetUpdate = Mono::CompileMethod("", "PlayerMain", "Update", 0);
      if (targetUpdate) {
        if (MH_CreateHook(targetUpdate, (void*) hkPlayerMainUpdate, (void**) &oPlayerMainUpdate) == MH_OK) {
          MH_EnableHook(targetUpdate);
          Logger::Log("[InfiniteStamina] Successfully hooked PlayerMain::Update!");
        }
        else {
          Logger::Log("[InfiniteStamina] Failed to hook PlayerMain::Update!");
        }
      }
      else {
        Logger::Log("[InfiniteStamina] Failed to compile PlayerMain::Update!");
      }

      staminaFastField = Mono::GetFieldFromName("", "PlayerMain", "staminaFast");
      staminaSlowField = Mono::GetFieldFromName("", "PlayerMain", "staminaSlow");
      maxStaminaField  = Mono::GetFieldFromName("", "PlayerMain", "maxStamina");
      if (staminaFastField && staminaSlowField && maxStaminaField) {
        Logger::Log("[InfiniteStamina] Successfully resolved stamina fields!");
      }
      else {
        Logger::Log("[InfiniteStamina] Failed to resolve some stamina fields!");
      }
    }
  }  // namespace Player
}  // namespace Features
