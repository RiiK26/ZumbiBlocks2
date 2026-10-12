#include "menu.hpp"
#include "config.hpp"
#include <imgui.h>
#include "../../features/player/player.hpp"
#include "../../features/combat/combat.hpp"
#include "../utils/mono_resolver.hpp"

#ifndef PROJECT_VERSION
  #define PROJECT_VERSION "UNKNOWN"
#endif

namespace UI
{
  void DrawMenuUI()
  {
    if (Config.bMenuOpen) {
      ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
      ImGui::Begin(
        "ZumbiBlocks2 v" PROJECT_VERSION, nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
      );

      if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
        if (ImGui::BeginTabItem("Player")) {
          ImGui::BeginChild("PlayerChild", ImVec2(0, -45), 0);
          ImGui::Checkbox("Invincible", &Config.bInvincible);
          ImGui::Checkbox("Infinite Stamina", &Config.bInfiniteStamina);

          ImGui::Spacing();
          ImGui::Separator();
          ImGui::Text("Loadout & Customization");
          ImGui::Checkbox("Auto Unlock Loadouts", &Config.bUnlockAllLoadouts);
          if (ImGui::Button("Unlock All Loadouts & Skins", ImVec2(-1, 0))) {
            Features::Player::UnlockAllLoadouts();
          }
          const char* unlockStatus = Features::Player::GetUnlockStatus();
          if (unlockStatus && unlockStatus[0] != '\0') {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", unlockStatus);
          }

          ImGui::EndChild();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Combat")) {
          ImGui::BeginChild("CombatChild", ImVec2(0, -45), 0);
          ImGui::Checkbox("One Hit Kill", &Config.bOneHitKill);
          ImGui::Checkbox("Infinite Ammo", &Config.bInfiniteAmmo);
          ImGui::Checkbox("Aura Kill", &Config.bAuraKill);
          ImGui::EndChild();
          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visuals")) {
          ImGui::BeginChild("VisualsChild", ImVec2(0, -45), 0);
          ImGui::Checkbox("Enable Crosshair", &Config.bEnableCrosshair);

          const char* crosshairTypes[] = {"Crosshair", "Dot Crosshair"};
          ImGui::Combo("Crosshair Type", &Config.iCrosshairType, crosshairTypes, IM_ARRAYSIZE(crosshairTypes));
          ImGui::EndChild();
          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

      ImGui::Separator();
      if (ImGui::Button("Save Config", ImVec2(-1, 0))) {
        Config.SaveConfig();
      }
      ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", "[INSERT] show/hide menu");
      ImGui::End();
    }

    // Call game logic updates safely on DX thread
    static int frameCounter = 0;
    frameCounter++;
    if (frameCounter % 5 == 0) {  // Run ~12 times a second for responsive cheats
      Features::Player::UpdateLoadoutUnlocker();

      void* playerInstance = Mono::GetLocalPlayer();
      if (playerInstance) {
        Features::Player::UpdateInvincible(playerInstance);
        Features::Combat::UpdateAuraKill(playerInstance);
        Features::Combat::UpdateAmmo(playerInstance);
        Features::Combat::UpdateOneHitKill(playerInstance);

        if (Config.bInfiniteStamina) {
          Mono::MonoField* g_healthFastField = Mono::GetFieldFromName("", "PlayerMain", "staminaFast");
          Mono::MonoField* g_healthSlowField = Mono::GetFieldFromName("", "PlayerMain", "staminaSlow");
          float            maxStamina        = 100.0f;  // Assumed max
          if (g_healthFastField)
            Mono::SetFieldValue((Mono::MonoObject*) playerInstance, g_healthFastField, &maxStamina);
          if (g_healthSlowField)
            Mono::SetFieldValue((Mono::MonoObject*) playerInstance, g_healthSlowField, &maxStamina);
        }
      }
    }

    // Draw Crosshairs
    if (Config.bEnableCrosshair) {
      ImGuiIO&    io = ImGui::GetIO();
      ImVec2      center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
      ImDrawList* drawList = ImGui::GetBackgroundDrawList();
      ImU32       color    = IM_COL32(0, 255, 0, 255);  // Green color

      if (Config.iCrosshairType == 1) {                 // Dot Crosshair
        drawList->AddCircleFilled(center, 2.0f, color);
      }
      else if (Config.iCrosshairType == 0) {  // Crosshair
        float size         = 8.0f;
        float gap          = 3.0f;
        float thickness    = 1.5f;

        // Draw outline (black) for better visibility
        ImU32 outlineColor = IM_COL32(0, 0, 0, 255);
        drawList->AddLine(
          ImVec2(center.x - size, center.y), ImVec2(center.x - gap, center.y), outlineColor, thickness + 1.0f
        );
        drawList->AddLine(
          ImVec2(center.x + gap, center.y), ImVec2(center.x + size, center.y), outlineColor, thickness + 1.0f
        );
        drawList->AddLine(
          ImVec2(center.x, center.y - size), ImVec2(center.x, center.y - gap), outlineColor, thickness + 1.0f
        );
        drawList->AddLine(
          ImVec2(center.x, center.y + gap), ImVec2(center.x, center.y + size), outlineColor, thickness + 1.0f
        );

        // Draw crosshair
        drawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x - gap, center.y), color, thickness);
        drawList->AddLine(ImVec2(center.x + gap, center.y), ImVec2(center.x + size, center.y), color, thickness);
        drawList->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y - gap), color, thickness);
        drawList->AddLine(ImVec2(center.x, center.y + gap), ImVec2(center.x, center.y + size), color, thickness);
      }
    }
  }
}  // namespace UI
