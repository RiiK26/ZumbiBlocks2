#include <windows.h>
#include "logger.hpp"

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

  // Exported function types
  typedef MonoThread*(__cdecl* mono_thread_attach_t)(MonoDomain* domain);
  typedef MonoDomain*(__cdecl* mono_get_root_domain_t)();
  typedef MonoDomain*(__cdecl* mono_domain_get_t)();
  typedef MonoAssembly*(__cdecl* mono_domain_assembly_open_t)(MonoDomain* domain, const char* name);
  typedef MonoAssembly*(__cdecl* mono_assembly_open_t)(const char* filename, int* status);
  typedef MonoImage*(__cdecl* mono_assembly_get_image_t)(MonoAssembly* assembly);
  typedef void(__cdecl* mono_assembly_foreach_t)(void (*func)(MonoAssembly*, void*), void* user_data);
  typedef struct MonoAssemblyName*(__cdecl* mono_assembly_get_name_t)(MonoAssembly* assembly);
  typedef const char*(__cdecl* mono_assembly_name_get_name_t)(struct MonoAssemblyName* aname);
  typedef MonoClass*(__cdecl* mono_class_from_name_t)(MonoImage* image, const char* name_space, const char* name);
  typedef MonoMethod*(__cdecl* mono_class_get_method_from_name_t)(MonoClass* klass, const char* name, int param_count);
  typedef MonoField*(__cdecl* mono_class_get_field_from_name_t)(MonoClass* klass, const char* name);
  typedef void*(__cdecl* mono_runtime_invoke_t)(MonoMethod* method, void* obj, void** params, MonoObject** exc);
  typedef void(__cdecl* mono_field_get_value_t)(MonoObject* obj, MonoField* field, void* value);
  typedef void(__cdecl* mono_field_set_value_t)(MonoObject* obj, MonoField* field, void* value);
  typedef void(__cdecl* mono_field_static_get_value_t)(MonoField* field, void* value);
  typedef void(__cdecl* mono_field_static_set_value_t)(MonoField* field, void* value);
  typedef char*(__cdecl* mono_string_to_utf8_t)(MonoString* string_obj);
  typedef MonoString*(__cdecl* mono_string_new_t)(MonoDomain* domain, const char* text);
  typedef void*(__cdecl* mono_compile_method_t)(MonoMethod* method);
  typedef void*(__cdecl* mono_object_unbox_t)(MonoObject* obj);
  typedef MonoType*(__cdecl* mono_class_get_type_t)(MonoClass* klass);
  typedef MonoClass*(__cdecl* mono_type_get_class_t)(MonoType* type);

  // Globals (Initialized once)
  mono_thread_attach_t              mono_thread_attach              = nullptr;
  mono_get_root_domain_t            mono_get_root_domain            = nullptr;
  mono_domain_get_t                 mono_domain_get                 = nullptr;
  mono_domain_assembly_open_t       mono_domain_assembly_open       = nullptr;
  mono_assembly_open_t              mono_assembly_open              = nullptr;
  mono_assembly_get_image_t         mono_assembly_get_image         = nullptr;
  mono_assembly_foreach_t           mono_assembly_foreach           = nullptr;
  mono_assembly_get_name_t          mono_assembly_get_name          = nullptr;
  mono_assembly_name_get_name_t     mono_assembly_name_get_name     = nullptr;
  mono_class_from_name_t            mono_class_from_name            = nullptr;
  mono_class_get_method_from_name_t mono_class_get_method_from_name = nullptr;
  mono_class_get_field_from_name_t  mono_class_get_field_from_name  = nullptr;
  mono_runtime_invoke_t             mono_runtime_invoke             = nullptr;
  mono_field_get_value_t            mono_field_get_value            = nullptr;
  mono_field_set_value_t            mono_field_set_value            = nullptr;
  mono_field_static_get_value_t     mono_field_static_get_value     = nullptr;
  mono_field_static_set_value_t     mono_field_static_set_value     = nullptr;
  mono_string_to_utf8_t             mono_string_to_utf8             = nullptr;
  mono_string_new_t                 mono_string_new                 = nullptr;
  mono_compile_method_t             mono_compile_method             = nullptr;
  mono_object_unbox_t               mono_object_unbox               = nullptr;
  mono_class_get_type_t             mono_class_get_type             = nullptr;
  mono_type_get_class_t             mono_type_get_class             = nullptr;

  bool Init()
  {
    HMODULE hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
    if (!hMono) {
      return false;
    }

    mono_thread_attach          = (mono_thread_attach_t) GetProcAddress(hMono, "mono_thread_attach");
    mono_get_root_domain        = (mono_get_root_domain_t) GetProcAddress(hMono, "mono_get_root_domain");
    mono_domain_get             = (mono_domain_get_t) GetProcAddress(hMono, "mono_domain_get");
    mono_domain_assembly_open   = (mono_domain_assembly_open_t) GetProcAddress(hMono, "mono_domain_assembly_open");
    mono_assembly_open          = (mono_assembly_open_t) GetProcAddress(hMono, "mono_assembly_open");
    mono_assembly_get_image     = (mono_assembly_get_image_t) GetProcAddress(hMono, "mono_assembly_get_image");
    mono_assembly_foreach       = (mono_assembly_foreach_t) GetProcAddress(hMono, "mono_assembly_foreach");
    mono_assembly_get_name      = (mono_assembly_get_name_t) GetProcAddress(hMono, "mono_assembly_get_name");
    mono_assembly_name_get_name = (mono_assembly_name_get_name_t) GetProcAddress(hMono, "mono_assembly_name_get_name");
    mono_class_from_name        = (mono_class_from_name_t) GetProcAddress(hMono, "mono_class_from_name");
    mono_class_get_method_from_name =
      (mono_class_get_method_from_name_t) GetProcAddress(hMono, "mono_class_get_method_from_name");
    mono_class_get_field_from_name =
      (mono_class_get_field_from_name_t) GetProcAddress(hMono, "mono_class_get_field_from_name");
    mono_runtime_invoke         = (mono_runtime_invoke_t) GetProcAddress(hMono, "mono_runtime_invoke");
    mono_field_get_value        = (mono_field_get_value_t) GetProcAddress(hMono, "mono_field_get_value");
    mono_field_set_value        = (mono_field_set_value_t) GetProcAddress(hMono, "mono_field_set_value");
    mono_field_static_get_value = (mono_field_static_get_value_t) GetProcAddress(hMono, "mono_field_static_get_value");
    mono_field_static_set_value = (mono_field_static_set_value_t) GetProcAddress(hMono, "mono_field_static_set_value");
    mono_string_to_utf8         = (mono_string_to_utf8_t) GetProcAddress(hMono, "mono_string_to_utf8");
    mono_string_new             = (mono_string_new_t) GetProcAddress(hMono, "mono_string_new");
    mono_compile_method         = (mono_compile_method_t) GetProcAddress(hMono, "mono_compile_method");
    mono_object_unbox           = (mono_object_unbox_t) GetProcAddress(hMono, "mono_object_unbox");
    mono_class_get_type         = (mono_class_get_type_t) GetProcAddress(hMono, "mono_class_get_type");
    mono_type_get_class         = (mono_type_get_class_t) GetProcAddress(hMono, "mono_type_get_class");

    // Verify the critical functions were successfully loaded
    return mono_thread_attach && mono_get_root_domain && mono_class_from_name;
  }

  void* CompileMethod(const char* name_space, const char* class_name, const char* method_name, int param_count)
  {
    if (!mono_get_root_domain || !mono_domain_assembly_open || !mono_assembly_get_image) {
      Logger::Log("[Mono] Critical functions missing in CompileMethod");
      return nullptr;
    }

    MonoDomain* domain = mono_get_root_domain();
    if (!domain) {
      Logger::Log("[Mono] Failed to get root domain");
      return nullptr;
    }

    if (mono_thread_attach)
      mono_thread_attach(domain);

    MonoAssembly* assembly = nullptr;
    if (mono_assembly_foreach && mono_assembly_get_name && mono_assembly_name_get_name) {
      struct SearchData
      {
        const char*   targetName;
        MonoAssembly* foundAssembly;
      } searchData = {"Assembly-CSharp", nullptr};

      mono_assembly_foreach(
        [](MonoAssembly* assem, void* user_data) {
          SearchData* data = (SearchData*) user_data;
          if (data->foundAssembly)
            return;
          struct MonoAssemblyName* aname = mono_assembly_get_name(assem);
          if (aname) {
            const char* name = mono_assembly_name_get_name(aname);
            if (name && strcmp(name, data->targetName) == 0) {
              data->foundAssembly = assem;
            }
          }
        },
        &searchData
      );

      assembly = searchData.foundAssembly;
    }

    if (!assembly) {
      assembly = mono_domain_assembly_open(domain, "Assembly-CSharp.dll");
      if (!assembly) {
        assembly = mono_domain_assembly_open(domain, "Assembly-CSharp");
      }
    }

    if (!assembly) {
      Logger::Log("[Mono] Failed to open Assembly-CSharp via iteration and open!");
      return nullptr;
    }

    MonoImage* image = mono_assembly_get_image(assembly);
    if (!image) {
      Logger::Log("[Mono] Failed to get image for Assembly-CSharp");
      return nullptr;
    }

    MonoClass* klass = mono_class_from_name(image, name_space, class_name);
    if (!klass) {
      Logger::Log("[Mono] Failed to find class %s::%s", name_space, class_name);
      return nullptr;
    }

    MonoMethod* method = mono_class_get_method_from_name(klass, method_name, param_count);
    if (!method) {
      Logger::Log("[Mono] Failed to find method %s with %d params", method_name, param_count);
      // Fallback: try finding method with any params to see if it exists
      MonoMethod* any_method = mono_class_get_method_from_name(klass, method_name, -1);
      if (any_method) {
        Logger::Log("[Mono] Found method %s but with different parameter count!", method_name);
      }
      return nullptr;
    }

    if (mono_compile_method) {
      void* compiled = mono_compile_method(method);
      if (!compiled)
        Logger::Log("[Mono] Failed to compile method %s", method_name);
      return compiled;
    }

    Logger::Log("[Mono] mono_compile_method is null");
    return nullptr;
  }
}  // namespace Mono
