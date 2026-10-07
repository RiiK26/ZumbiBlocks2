#pragma once

#include <windows.h>
#include <string>  // IWYU pragma: keep

namespace Mono
{
  // Mono types
  struct MonoDomain;
  struct MonoAssembly;
  struct MonoImage;
  struct MonoClass;
  struct MonoMethod;
  struct MonoProperty;
  struct MonoObject;
  struct MonoString;
  struct MonoType;
  struct MonoField;
  struct MonoThread;

  // Globals
  extern bool Init();

  // Helper function to resolve a method and compile it to a native pointer
  extern void* CompileMethod(const char* name_space, const char* class_name, const char* method_name, int param_count);

}  // namespace Mono
