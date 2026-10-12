#include "player.hpp"
#include "../../modules/ui/config.hpp"
#include "../../modules/utils/mono_resolver.hpp"

#include <cmath>
#include <imgui.h>
#include <windows.h>

namespace Features
{
  namespace Player
  {
    struct Vector3
    {
      float x, y, z;
    };

    // Cached Mono Fields
    static Mono::MonoField*  g_PlayerMovementField         = nullptr;
    static Mono::MonoField*  g_PlayerCapsuleColField       = nullptr;
    static Mono::MonoField*  g_PlayerCamField              = nullptr;
    static Mono::MonoField*  g_MovementBodyField           = nullptr;
    static Mono::MonoField*  g_MovementHitboxField         = nullptr;
    static Mono::MonoField*  g_MovementTouchingGroundField = nullptr;
    static Mono::MonoField*  g_MovementStateField          = nullptr;
    static Mono::MonoField*  g_CamRaycastMaskField         = nullptr;

    // Cached Mono Methods
    static Mono::MonoMethod* g_ComponentGetTransform       = nullptr;
    static Mono::MonoMethod* g_CamGetCameraTransform       = nullptr;
    static Mono::MonoMethod* g_TransformGetForward         = nullptr;
    static Mono::MonoMethod* g_TransformGetRight           = nullptr;

    static Mono::MonoMethod* g_ColliderSetEnabled          = nullptr;
    static Mono::MonoMethod* g_RbSetUseGravity             = nullptr;
    static Mono::MonoMethod* g_RbSetLinearVelocity         = nullptr;
    static Mono::MonoMethod* g_RbSetVelocity               = nullptr;

    // State trackers
    static bool              g_WasNoClipActive             = false;
    static void*             g_LastPlayerInstance          = nullptr;
    static int               g_OriginalRaycastMask         = 0;
    static bool              g_HasOriginalRaycastMask      = false;

    void InitializeNoClip()
    {
      g_PlayerMovementField         = Mono::GetFieldFromName("", "PlayerMain", "movement");
      g_PlayerCapsuleColField       = Mono::GetFieldFromName("", "PlayerMain", "capsuleCollider");
      g_PlayerCamField              = Mono::GetFieldFromName("", "PlayerMain", "cam");

      g_MovementBodyField           = Mono::GetFieldFromName("", "PlayerMovement", "body");
      g_MovementHitboxField         = Mono::GetFieldFromName("", "PlayerMovement", "hitbox");
      g_MovementTouchingGroundField = Mono::GetFieldFromName("", "PlayerMovement", "touchingGround");
      g_MovementStateField          = Mono::GetFieldFromName("", "PlayerMovement", "state");

      g_CamRaycastMaskField         = Mono::GetFieldFromName("", "PlayerCamera", "raycastMask");

      g_ComponentGetTransform       = Mono::GetMethodFromName("UnityEngine", "Component", "get_transform", 0);
      g_CamGetCameraTransform       = Mono::GetMethodFromName("", "PlayerCamera", "get_CameraTransform", 0);
      g_TransformGetForward         = Mono::GetMethodFromName("UnityEngine", "Transform", "get_forward", 0);
      g_TransformGetRight           = Mono::GetMethodFromName("UnityEngine", "Transform", "get_right", 0);

      g_ColliderSetEnabled          = Mono::GetMethodFromName("UnityEngine", "Collider", "set_enabled", 1);
      g_RbSetUseGravity             = Mono::GetMethodFromName("UnityEngine", "Rigidbody", "set_useGravity", 1);
      g_RbSetLinearVelocity         = Mono::GetMethodFromName("UnityEngine", "Rigidbody", "set_linearVelocity", 1);
      g_RbSetVelocity               = Mono::GetMethodFromName("UnityEngine", "Rigidbody", "set_velocity", 1);
    }

    void UpdateNoClip(void* playerInstance)
    {
      if (!playerInstance) {
        if (g_WasNoClipActive) {
          RestoreNoClip(g_LastPlayerInstance);
        }
        return;
      }

      if (!g_PlayerMovementField) {
        InitializeNoClip();
        if (!g_PlayerMovementField)
          return;
      }

      if (playerInstance != g_LastPlayerInstance) {
        if (g_WasNoClipActive && g_LastPlayerInstance) {
          RestoreNoClip(g_LastPlayerInstance);
        }
        g_LastPlayerInstance     = playerInstance;
        g_HasOriginalRaycastMask = false;
      }

      // Extract components
      void* movementObj   = nullptr;
      void* capsuleColObj = nullptr;
      void* camObj        = nullptr;
      if (g_PlayerMovementField)
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerMovementField, &movementObj);
      if (g_PlayerCapsuleColField)
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerCapsuleColField, &capsuleColObj);
      if (g_PlayerCamField)
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerCamField, &camObj);

      void* bodyObj   = nullptr;
      void* hitboxObj = nullptr;
      if (movementObj) {
        if (g_MovementBodyField)
          Mono::GetFieldValue((Mono::MonoObject*) movementObj, g_MovementBodyField, &bodyObj);
        if (g_MovementHitboxField)
          Mono::GetFieldValue((Mono::MonoObject*) movementObj, g_MovementHitboxField, &hitboxObj);
      }

      if (UI::Config.bNoClip) {
        g_WasNoClipActive    = true;

        // 1. Disable player colliders to pass freely through walls, doors, and objects
        bool  bDisabled      = false;
        void* argDisabled[1] = {&bDisabled};
        if (hitboxObj && g_ColliderSetEnabled)
          Mono::RuntimeInvoke(g_ColliderSetEnabled, hitboxObj, argDisabled);
        if (capsuleColObj && g_ColliderSetEnabled)
          Mono::RuntimeInvoke(g_ColliderSetEnabled, capsuleColObj, argDisabled);

        // 2. Disable physics gravity so the player never falls through the floor
        if (bodyObj && g_RbSetUseGravity)
          Mono::RuntimeInvoke(g_RbSetUseGravity, bodyObj, argDisabled);

        // 3. Keep touchingGround = true and state = Walk so the character animations stay active
        if (movementObj) {
          if (g_MovementTouchingGroundField) {
            bool bTrue = true;
            Mono::SetFieldValue((Mono::MonoObject*) movementObj, g_MovementTouchingGroundField, &bTrue);
          }
          if (g_MovementStateField) {
            int walkState = 1;  // State.Walk
            Mono::SetFieldValue((Mono::MonoObject*) movementObj, g_MovementStateField, &walkState);
          }
        }

        // 4. Set camera raycastMask to 0 so the camera never clips against walls behind the player
        if (camObj && g_CamRaycastMaskField) {
          if (!g_HasOriginalRaycastMask) {
            Mono::GetFieldValue((Mono::MonoObject*) camObj, g_CamRaycastMaskField, &g_OriginalRaycastMask);
            g_HasOriginalRaycastMask = true;
          }
          int zeroMask = 0;
          Mono::SetFieldValue((Mono::MonoObject*) camObj, g_CamRaycastMaskField, &zeroMask);
        }

        // 5. Determine horizontal movement orientation from camera / player
        void* playerTransform = nullptr;
        if (g_ComponentGetTransform) {
          playerTransform = Mono::RuntimeInvoke(g_ComponentGetTransform, playerInstance);
        }

        void* camTransform = nullptr;
        if (camObj && g_CamGetCameraTransform) {
          camTransform = Mono::RuntimeInvoke(g_CamGetCameraTransform, camObj);
        }
        if (!camTransform && camObj && g_ComponentGetTransform) {
          camTransform = Mono::RuntimeInvoke(g_ComponentGetTransform, camObj);
        }
        if (!camTransform) {
          camTransform = playerTransform;
        }

        Vector3 forward = {0.0f, 0.0f, 1.0f};
        Vector3 right   = {1.0f, 0.0f, 0.0f};

        if (camTransform && Mono::mono_object_unbox) {
          if (g_TransformGetForward) {
            void* res = Mono::RuntimeInvoke(g_TransformGetForward, camTransform);
            if (res)
              forward = *(Vector3*) Mono::mono_object_unbox((Mono::MonoObject*) res);
          }
          if (g_TransformGetRight) {
            void* res = Mono::RuntimeInvoke(g_TransformGetRight, camTransform);
            if (res)
              right = *(Vector3*) Mono::mono_object_unbox((Mono::MonoObject*) res);
          }
        }

        // Flatten forward and right to the horizontal plane (X-Z) so looking up/down doesn't affect horizontal speed
        forward.y  = 0.0f;
        float fMag = std::sqrt((forward.x * forward.x) + (forward.z * forward.z));
        if (fMag > 0.0001f) {
          forward.x /= fMag;
          forward.z /= fMag;
        }

        right.y    = 0.0f;
        float rMag = std::sqrt((right.x * right.x) + (right.z * right.z));
        if (rMag > 0.0001f) {
          right.x /= rMag;
          right.z /= rMag;
        }

        // 6. Compute target velocity from user inputs
        ImGuiIO& io        = ImGui::GetIO();
        Vector3  targetVel = {0.0f, 0.0f, 0.0f};

        float    speed     = UI::Config.fNoClipSpeed;
        if (!io.WantCaptureKeyboard && (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
          speed *= 1.8f;  // Sprint boost
        }

        if (!io.WantCaptureKeyboard) {
          // Horizontal movement (W/S/A/D)
          if (GetAsyncKeyState('W') & 0x8000) {
            targetVel.x += forward.x * speed;
            targetVel.z += forward.z * speed;
          }
          if (GetAsyncKeyState('S') & 0x8000) {
            targetVel.x -= forward.x * speed;
            targetVel.z -= forward.z * speed;
          }
          if (GetAsyncKeyState('D') & 0x8000) {
            targetVel.x += right.x * speed;
            targetVel.z += right.z * speed;
          }
          if (GetAsyncKeyState('A') & 0x8000) {
            targetVel.x -= right.x * speed;
            targetVel.z -= right.z * speed;
          }

          // Normalize diagonal horizontal velocity so strafing doesn't exceed target speed
          float hMagSq = (targetVel.x * targetVel.x) + (targetVel.z * targetVel.z);
          if (hMagSq > (speed * speed)) {
            float hMag  = std::sqrt(hMagSq);
            targetVel.x = (targetVel.x / hMag) * speed;
            targetVel.z = (targetVel.z / hMag) * speed;
          }

          // Vertical flight (Space = ascend, Ctrl/C = descend, neither = hover at current height)
          if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            targetVel.y = speed;
          }
          else if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState('C') & 0x8000)) {
            targetVel.y = -speed;
          }
          else {
            targetVel.y = 0.0f;  // Hover
          }
        }

        // 7. Apply velocity to Rigidbody
        if (bodyObj) {
          void* argVel[1] = {&targetVel};
          if (g_RbSetLinearVelocity)
            Mono::RuntimeInvoke(g_RbSetLinearVelocity, bodyObj, argVel);
          else if (g_RbSetVelocity)
            Mono::RuntimeInvoke(g_RbSetVelocity, bodyObj, argVel);
        }
      }
      else {
        // Disabled: restore original physics, colliders, and camera if was active
        if (g_WasNoClipActive) {
          RestoreNoClip(playerInstance);
        }
      }
    }

    void RestoreNoClip(void* playerInstance)
    {
      if (!playerInstance)
        return;

      g_WasNoClipActive   = false;

      void* movementObj   = nullptr;
      void* capsuleColObj = nullptr;
      void* camObj        = nullptr;
      if (g_PlayerMovementField)
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerMovementField, &movementObj);
      if (g_PlayerCapsuleColField)
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerCapsuleColField, &capsuleColObj);
      if (g_PlayerCamField)
        Mono::GetFieldValue((Mono::MonoObject*) playerInstance, g_PlayerCamField, &camObj);

      void* bodyObj   = nullptr;
      void* hitboxObj = nullptr;
      if (movementObj) {
        if (g_MovementBodyField)
          Mono::GetFieldValue((Mono::MonoObject*) movementObj, g_MovementBodyField, &bodyObj);
        if (g_MovementHitboxField)
          Mono::GetFieldValue((Mono::MonoObject*) movementObj, g_MovementHitboxField, &hitboxObj);
      }

      // Re-enable colliders
      bool  bEnabled      = true;
      void* argEnabled[1] = {&bEnabled};
      if (hitboxObj && g_ColliderSetEnabled)
        Mono::RuntimeInvoke(g_ColliderSetEnabled, hitboxObj, argEnabled);
      if (capsuleColObj && g_ColliderSetEnabled)
        Mono::RuntimeInvoke(g_ColliderSetEnabled, capsuleColObj, argEnabled);

      // Re-enable gravity
      if (bodyObj && g_RbSetUseGravity)
        Mono::RuntimeInvoke(g_RbSetUseGravity, bodyObj, argEnabled);

      // Reset velocity
      if (bodyObj) {
        Vector3 zeroVel   = {0.0f, 0.0f, 0.0f};
        void*   argVel[1] = {&zeroVel};
        if (g_RbSetLinearVelocity)
          Mono::RuntimeInvoke(g_RbSetLinearVelocity, bodyObj, argVel);
        else if (g_RbSetVelocity)
          Mono::RuntimeInvoke(g_RbSetVelocity, bodyObj, argVel);
      }

      // Restore camera raycast mask
      if (camObj && g_CamRaycastMaskField && g_HasOriginalRaycastMask) {
        Mono::SetFieldValue((Mono::MonoObject*) camObj, g_CamRaycastMaskField, &g_OriginalRaycastMask);
      }

      // Restore movement state to Walk (State.Walk = 1)
      if (movementObj && g_MovementStateField) {
        int walkState = 1;
        Mono::SetFieldValue((Mono::MonoObject*) movementObj, g_MovementStateField, &walkState);
      }
    }
  }  // namespace Player
}  // namespace Features
