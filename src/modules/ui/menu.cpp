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
          ImGui::EndChild();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Combat")) {
          ImGui::BeginChild("CombatChild", ImVec2(0, -45), 0);
          ImGui::Text("Combat features coming soon...");
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
  }
}  // namespace UI
