#include "menu.hpp"
#include "config.hpp"
#include <imgui.h>

#ifndef PROJECT_VERSION
  #define PROJECT_VERSION "UNKNOWN"
#endif

namespace UI
{
  void DrawMenuUI()
  {
    if (Config.bMenuOpen) {
      ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
      ImGui::Begin(
        "ZumbiBlocks2 v" PROJECT_VERSION, nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
      );

      if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
        if (ImGui::BeginTabItem("Player")) {
          ImGui::BeginChild("PlayerChild", ImVec2(0, -45), 0);
          ImGui::Checkbox("Invincible", &Config.bInvincible);
          ImGui::Checkbox("Infinite Stamina", &Config.bInfiniteStamina);
          ImGui::EndChild();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Combat")) {
          ImGui::BeginChild("CombatChild", ImVec2(0, -45), 0);
          if (ImGui::Checkbox("One Hit Kill", &Config.bOneHitKill)) {
            Config.SaveConfig();
          }
          if (ImGui::Checkbox("Infinite Ammo", &Config.bInfiniteAmmo)) {
            Config.SaveConfig();
          }
          ImGui::EndChild();
          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visuals")) {
          ImGui::BeginChild("VisualsChild", ImVec2(0, -45), 0);
          if (ImGui::Checkbox("Crosshair", &Config.bCrosshair))
            Config.SaveConfig();
          if (ImGui::Checkbox("Dot Crosshair", &Config.bDotCrosshair))
            Config.SaveConfig();
          ImGui::EndChild();
          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }

      ImGui::Separator();
      if (ImGui::Button("Save Config", ImVec2(-1, 0))) {
        Config.SaveConfig();
      }
      ImGui::End();
    }

    // Draw Crosshairs
    if (Config.bCrosshair || Config.bDotCrosshair) {
      ImGuiIO&    io = ImGui::GetIO();
      ImVec2      center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
      ImDrawList* drawList = ImGui::GetBackgroundDrawList();
      ImU32       color    = IM_COL32(0, 255, 0, 255);  // Green color

      if (Config.bDotCrosshair) {
        drawList->AddCircleFilled(center, 2.0f, color);
      }

      if (Config.bCrosshair) {
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
