#pragma once
#include <windows.h>

namespace Scanner
{
  uintptr_t FindPattern(HMODULE hModule, const char* pattern);
}  // namespace Scanner
