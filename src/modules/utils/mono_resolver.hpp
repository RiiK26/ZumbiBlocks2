#pragma once

#include <windows.h>
#include <string>  // IWYU pragma: keep
#include <vector>

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
  extern MonoClass* GetClassFromName(const char* name_space, const char* class_name);
  extern MonoDomain* GetRootDomain();
  extern MonoDomain* GetActiveDomain();
  extern void EnsureThreadAttached();
  extern void* FindObjectOfType(const char* name_space, const char* class_name);
  extern void* GetLocalPlayer();
  extern bool  IsInGame();

  // Raw Mono API exports
  extern void* (*mono_runtime_invoke)(MonoMethod* method, void* obj, void** params, MonoObject** exc);
  extern void* (*mono_object_unbox)(MonoObject* obj);
  extern void* (*mono_object_get_class)(MonoObject* obj);
  extern MonoObject* (*mono_object_new)(MonoDomain* domain, MonoClass* klass);
  extern MonoMethod* (*mono_class_get_method_from_name)(MonoClass* klass, const char* name, int param_count);
  extern void* (*mono_type_get_object)(MonoDomain* domain, MonoType* type);

  // Method & Field Helpers
  extern MonoMethod*
  GetMethodFromName(const char* name_space, const char* class_name, const char* method_name, int param_count);
  extern void* RuntimeInvoke(MonoMethod* method, void* obj, void** params = nullptr, MonoObject** exc = nullptr);
  extern MonoField* GetFieldFromName(const char* name_space, const char* class_name, const char* field_name);
  extern void GetFieldValue(MonoObject* obj, MonoField* field, void* value);
  extern void SetFieldValue(MonoObject* obj, MonoField* field, void* value);
  extern void GetStaticFieldValue(MonoField* field, void* value);
  extern void SetStaticFieldValue(MonoField* field, void* value);

  // List Helpers
  extern bool GetListElements(void* listObj, std::vector<void*>& outElements);
  extern void* GetListItem(void* listObj, int index);
  extern int GetListCount(void* listObj);

  extern void DumpMethods(const char* name_space, const char* class_name);
  extern void DumpFields(const char* name_space, const char* class_name);

}  // namespace Mono
