#include "player.hpp"
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/mono_resolver.hpp"

#include <cmath>

namespace Features
{
  namespace Player
  {
    static Mono::MonoField* g_MovementField     = nullptr;
    static Mono::MonoField* g_WalkSpeedField    = nullptr;
    static float            g_OriginalWalkSpeed = 0.0f;
    static void*            g_LastPlayer        = nullptr;

    void InitializeMovementSpeed()
    {
      g_MovementField  = Mono::GetFieldFromName("", "PlayerMain", "movement");
      g_WalkSpeedField = Mono::GetFieldFromName("", "PlayerMovement", "walkSpeed");
    }

    void UpdateMovementSpeed(void* playerInstance)
    {
      if (!playerInstance)
        return;

      if (!g_MovementField || !g_WalkSpeedField) {
        InitializeMovementSpeed();
        if (!g_MovementField || !g_WalkSpeedField)
          return;
      }

      void* movementObj = nullptr;
      Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_MovementField, &movementObj);
      if (!movementObj)
        return;

      if (playerInstance != g_LastPlayer) {
        g_LastPlayer        = playerInstance;
        g_OriginalWalkSpeed = 0.0f;
      }

      float currentWalkSpeed = 0.0f;
      Mono::GetFieldValue((Mono::MonoObject*) movementObj, g_WalkSpeedField, &currentWalkSpeed);

      if (UI::Config.bMovementSpeed) {
        // Record natural walk speed on first read
        if (g_OriginalWalkSpeed <= 0.0f && currentWalkSpeed > 0.0f) {
          g_OriginalWalkSpeed = currentWalkSpeed;
        }

        if (g_OriginalWalkSpeed > 0.0f) {
          float targetSpeed = g_OriginalWalkSpeed * UI::Config.fMovementSpeedMultiplier;
          if (std::abs(currentWalkSpeed - targetSpeed) > 0.01f) {
            Mono::SetFieldValue((Mono::MonoObject*) movementObj, g_WalkSpeedField, &targetSpeed);
          }
        }
      }
      else {
        // Restore original walk speed when disabled
        if (g_OriginalWalkSpeed > 0.0f && std::abs(currentWalkSpeed - g_OriginalWalkSpeed) > 0.01f) {
          Mono::SetFieldValue((Mono::MonoObject*) movementObj, g_WalkSpeedField, &g_OriginalWalkSpeed);
        }
      }
    }

    void RestoreMovementSpeed(void* playerInstance)
    {
      if (!playerInstance || !g_MovementField || !g_WalkSpeedField || g_OriginalWalkSpeed <= 0.0f)
        return;

      void* movementObj = nullptr;
      Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_MovementField, &movementObj);
      if (movementObj) {
        Mono::SetFieldValue((Mono::MonoObject*) movementObj, g_WalkSpeedField, &g_OriginalWalkSpeed);
      }
    }
  }  // namespace Player
}  // namespace Features
