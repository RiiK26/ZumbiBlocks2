#include "logger.hpp"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

namespace Logger
{
  static std::string logPath;

  void Initialize()
  {
    char    path[MAX_PATH];
    HMODULE hMod = NULL;

    GetModuleHandleExA(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      reinterpret_cast<LPCSTR>(&Initialize), &hMod
    );
    if (hMod && GetModuleFileNameA(hMod, path, MAX_PATH)) {
      std::string fullPath(path);
      size_t      lastSlash = fullPath.find_last_of("\\/");
      if (lastSlash != std::string::npos) {
        logPath = fullPath.substr(0, lastSlash + 1) + "ZumbiBlocks2.log";
      }
    }

    if (logPath.empty()) {
      logPath = "ZumbiBlocks2.log";
    }

    // Open file in 'w' mode to clear previous content
    FILE* f = fopen(logPath.c_str(), "w");
    if (f) {
      fprintf(f, "[Logger] Initialized\n");
      fclose(f);
    }
  }

  void Log(const char* format, ...)
  {
    if (logPath.empty())
      return;

    FILE* f = fopen(logPath.c_str(), "a");
    if (f) {
      va_list args;
      va_start(args, format);
      vfprintf(f, format, args);
      va_end(args);
      fprintf(f, "\n");
      fclose(f);
    }
  }
}  // namespace Logger
