#include "player.hpp"
#include "../../modules/hooks/hook.hpp"  // IWYU pragma: keep
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/logger.hpp"
#include "../../modules/utils/mono_resolver.hpp"

Mono::MonoField* g_healthFastField = nullptr;
Mono::MonoField* g_healthSlowField = nullptr;

namespace Features
{
  namespace Player
  {
    void InitializeInvicible()
    {
      Logger::Log("[Invincible] Initializing Health Updater...");
      g_healthFastField = Mono::GetFieldFromName("", "PlayerMain", "healthFast");
      g_healthSlowField = Mono::GetFieldFromName("", "PlayerMain", "healthSlow");
    }

    void UpdateInvincible(void* playerInstance)
    {
      if (UI::Config.bInvincible && playerInstance && g_healthFastField && g_healthSlowField) {
        float maxHealth = 100.0f;  // Could read MaxHealth, but 100 is standard
        Mono::SetFieldValue((Mono::MonoObject*) playerInstance, g_healthFastField, &maxHealth);
        Mono::SetFieldValue((Mono::MonoObject*) playerInstance, g_healthSlowField, &maxHealth);
      }
    }

    void UninitializeInvicible() { }
  }  // namespace Player
}  // namespace Features
