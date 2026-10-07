/*
HOOK_METHOD:    using Mono Resolver @src/modules/utils/mono_resolver.hpp
HOOK_SIGNATURE: using Signature scanner from @src/core/scanner.hpp
HOOK_OFFSET:    using Offset Scanner
*/

#pragma once

namespace Hooks
{
  extern bool bHooksFailed;
}

#include "../utils/mono_resolver.hpp"  // IWYU pragma: keep
#include "MinHook.h"                   // IWYU pragma: keep
#include "../../core/scanner.hpp"      // IWYU pragma: keep
#include <windows.h>

#define HOOK_METHOD(NameSpace, ClassName, MethodName, ArgsCount, HookFunc, OrigFuncPtr) \
  do { \
    void* target = Mono::CompileMethod(NameSpace, ClassName, MethodName, ArgsCount); \
    if (target) { \
      int createStatus = MH_CreateHook(target, (LPVOID) HookFunc, (LPVOID*) &OrigFuncPtr); \
      if (createStatus != MH_OK) { \
        Hooks::bHooksFailed = true; \
      } \
      else { \
        int enableStatus = MH_EnableHook(target); \
        if (enableStatus != MH_OK) { \
          Hooks::bHooksFailed = true; \
        } \
      } \
    } \
    else { \
      Hooks::bHooksFailed = true; \
    } \
  } while (0)

#define HOOK_SIGNATURE(ModuleName, Signature, HookFunc, OrigFuncPtr) \
  do { \
    void* target = (void*) Scanner::FindPattern(GetModuleHandleA(ModuleName), Signature); \
    if (target) { \
      int createStatus = MH_CreateHook(target, (LPVOID) HookFunc, (LPVOID*) &OrigFuncPtr); \
      if (createStatus != MH_OK) { \
        Hooks::bHooksFailed = true; \
      } \
      else { \
        int enableStatus = MH_EnableHook(target); \
        if (enableStatus != MH_OK) { \
          Hooks::bHooksFailed = true; \
        } \
      } \
    } \
    else { \
      Hooks::bHooksFailed = true; \
    } \
  } while (0)

#define HOOK_OFFSET(ModuleName, RVA, HookFunc, OrigFuncPtr) \
  do { \
    HMODULE hMod = GetModuleHandleA(ModuleName); \
    if (hMod) { \
      void* target       = (void*) ((uintptr_t) hMod + RVA); \
      int   createStatus = MH_CreateHook(target, (LPVOID) HookFunc, (LPVOID*) &OrigFuncPtr); \
      if (createStatus != MH_OK) { \
        Hooks::bHooksFailed = true; \
      } \
      else { \
        int enableStatus = MH_EnableHook(target); \
        if (enableStatus != MH_OK) { \
          Hooks::bHooksFailed = true; \
        } \
      } \
    } \
    else { \
      Hooks::bHooksFailed = true; \
    } \
  } while (0)

namespace Hooks
{
  void Initialize();
  void Uninitialize();
}  // namespace Hooks
