#pragma once
#include <string>  // IWYU pragma: keep

namespace Logger
{
  void Initialize();
  void Log(const char* format, ...);
}  // namespace Logger
