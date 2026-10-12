#include "config.hpp"
#include <windows.h>
#include <fstream>
#include <sstream>

namespace UI
{
  ConfigData Config;

  std::string GetModuleDirectory()
  {
    char    path[MAX_PATH];
    HMODULE hMod = NULL;

    GetModuleHandleExA(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      reinterpret_cast<LPCSTR>(&GetModuleDirectory), &hMod
    );
    if (hMod && GetModuleFileNameA(hMod, path, MAX_PATH)) {
      std::string fullPath(path);
      size_t      lastSlash = fullPath.find_last_of("\\/");
      if (lastSlash != std::string::npos) {
        return fullPath.substr(0, lastSlash + 1);
      }
    }
    return "";
  }

  std::string GetConfigPath()
  {
    std::string dir = GetModuleDirectory();
    return dir.empty() ? "config.txt" : dir + "config.txt";
  }

  void ConfigData::LoadConfig()
  {
    std::ifstream f(GetConfigPath());
    if (!f.is_open())
      return;

    std::string line;
    while (std::getline(f, line)) {
      std::istringstream is_line(line);
      std::string        key;
      if (std::getline(is_line, key, '=')) {
        std::string value;
        if (std::getline(is_line, value)) {
          if (key == "menu_open")
            bMenuOpen = (value == "1");
          else if (key == "invincible")
            bInvincible = (value == "1");
          else if (key == "infinite_stamina")
            bInfiniteStamina = (value == "1");
          else if (key == "one_hit_kill")
            bOneHitKill = (value == "1");
          else if (key == "infinite_ammo")
            bInfiniteAmmo = (value == "1");
          else if (key == "enable_crosshair")
            bEnableCrosshair = (value == "1");
          else if (key == "crosshair_type")
            iCrosshairType = std::stoi(value);
          else if (key == "aura_kill")
            bAuraKill = (value == "1");
        }
      }
    }
  }

  void ConfigData::SaveConfig()
  {
    std::ofstream out(GetConfigPath());
    if (!out.is_open())
      return;

    out << "menu_open=" << (bMenuOpen ? "1" : "0") << "\n";
    out << "invincible=" << (bInvincible ? "1" : "0") << "\n";
    out << "infinite_stamina=" << (bInfiniteStamina ? "1" : "0") << "\n";
    out << "one_hit_kill=" << (bOneHitKill ? "1" : "0") << "\n";
    out << "infinite_ammo=" << (bInfiniteAmmo ? "1" : "0") << "\n";
    out << "enable_crosshair=" << (bEnableCrosshair ? "1" : "0") << "\n";
    out << "crosshair_type=" << iCrosshairType << "\n";
    out << "aura_kill=" << (bAuraKill ? "1" : "0") << "\n";
  }
}  // namespace UI
