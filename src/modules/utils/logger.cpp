#include "logger.hpp"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

namespace Logger
{
  static std::string      logPath;
  static CRITICAL_SECTION logCs;
  static bool             logCsInit = false;

  void Initialize()
  {
    if (!logCsInit) {
      InitializeCriticalSection(&logCs);
      logCsInit = true;
    }

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
      fflush(f);
      fclose(f);
    }
  }

  void Log(const char* format, ...)
  {
    if (logPath.empty())
      return;

    if (logCsInit)
      EnterCriticalSection(&logCs);

    FILE* f = fopen(logPath.c_str(), "a");
    if (f) {
      va_list args;
      va_start(args, format);
      vfprintf(f, format, args);
      va_end(args);
      fprintf(f, "\n");
      fflush(f);
      fclose(f);
    }

    if (logCsInit)
      LeaveCriticalSection(&logCs);
  }

  void LogCrash(const char* format, ...)
  {
    char    buffer[2048];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer) - 2, format, args);
    va_end(args);
    if (len < 0)
      return;

    buffer[len]     = '\n';
    buffer[len + 1] = '\0';
    len++;

    HANDLE hFile = CreateFileA(
      logPath.empty() ? "ZumbiBlocks2.log" : logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL
    );
    if (hFile != INVALID_HANDLE_VALUE) {
      DWORD written = 0;
      WriteFile(hFile, buffer, (DWORD) len, &written, NULL);
      FlushFileBuffers(hFile);
      CloseHandle(hFile);
    }
  }
}  // namespace Logger
