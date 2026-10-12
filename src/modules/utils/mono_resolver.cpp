#include <windows.h>
#include <vector>
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
  struct MonoVTable;

  // Exported function types
  typedef void (*MonoDomainFunc)(MonoDomain* domain, void* user_data);
  typedef void(__cdecl* mono_domain_foreach_t)(MonoDomainFunc func, void* user_data);
  typedef int(__cdecl* mono_domain_set_t)(MonoDomain* domain, int force);
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
  typedef MonoField*(__cdecl* mono_class_get_fields_t)(MonoClass* klass, void** iter);
  typedef const char*(__cdecl* mono_field_get_name_t)(MonoField* field);
  typedef void*(__cdecl* mono_runtime_invoke_t)(MonoMethod* method, void* obj, void** params, MonoObject** exc);
  typedef void(__cdecl* mono_field_get_value_t)(MonoObject* obj, MonoField* field, void* value);
  typedef void(__cdecl* mono_field_set_value_t)(MonoObject* obj, MonoField* field, void* value);
  typedef void(__cdecl* mono_field_static_get_value_t)(MonoVTable* vt, MonoField* field, void* value);
  typedef void(__cdecl* mono_field_static_set_value_t)(MonoVTable* vt, MonoField* field, void* value);
  typedef MonoClass*(__cdecl* mono_field_get_parent_t)(MonoField* field);
  typedef MonoVTable*(__cdecl* mono_class_vtable_t)(MonoDomain* domain, MonoClass* klass);
  typedef char*(__cdecl* mono_string_to_utf8_t)(MonoString* string_obj);
  typedef MonoString*(__cdecl* mono_string_new_t)(MonoDomain* domain, const char* text);
  typedef void*(__cdecl* mono_compile_method_t)(MonoMethod* method);
  typedef void*(__cdecl* mono_object_unbox_t)(MonoObject* obj);
  typedef MonoClass*(__cdecl* mono_object_get_class_t)(MonoObject* obj);
  typedef MonoObject*(__cdecl* mono_object_new_t)(MonoDomain* domain, MonoClass* klass);
  typedef MonoType*(__cdecl* mono_class_get_type_t)(MonoClass* klass);
  typedef MonoClass*(__cdecl* mono_type_get_class_t)(MonoType* type);
  typedef MonoObject*(__cdecl* mono_type_get_object_t)(MonoDomain* domain, MonoType* type);

  // Globals (Initialized once)
  mono_domain_foreach_t             mono_domain_foreach             = nullptr;
  mono_domain_set_t                 mono_domain_set                 = nullptr;
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
  mono_class_get_fields_t           mono_class_get_fields           = nullptr;
  mono_field_get_name_t             mono_field_get_name             = nullptr;
  mono_runtime_invoke_t             mono_runtime_invoke             = nullptr;
  mono_field_get_value_t            mono_field_get_value            = nullptr;
  mono_field_set_value_t            mono_field_set_value            = nullptr;
  mono_field_static_get_value_t     mono_field_static_get_value     = nullptr;
  mono_field_static_set_value_t     mono_field_static_set_value     = nullptr;
  mono_field_get_parent_t           mono_field_get_parent           = nullptr;
  mono_class_vtable_t               mono_class_vtable               = nullptr;
  mono_string_to_utf8_t             mono_string_to_utf8             = nullptr;
  mono_string_new_t                 mono_string_new                 = nullptr;
  mono_compile_method_t             mono_compile_method             = nullptr;
  mono_object_unbox_t               mono_object_unbox               = nullptr;
  mono_object_get_class_t           mono_object_get_class           = nullptr;
  mono_object_new_t                 mono_object_new                 = nullptr;
  mono_class_get_type_t             mono_class_get_type             = nullptr;
  mono_type_get_class_t             mono_type_get_class             = nullptr;
  mono_type_get_object_t            mono_type_get_object            = nullptr;

  bool Init()
  {
    HMODULE hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
    if (!hMono) {
      return false;
    }

    mono_domain_foreach         = (mono_domain_foreach_t) GetProcAddress(hMono, "mono_domain_foreach");
    mono_domain_set             = (mono_domain_set_t) GetProcAddress(hMono, "mono_domain_set");
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
    mono_class_get_fields       = (mono_class_get_fields_t) GetProcAddress(hMono, "mono_class_get_fields");
    mono_field_get_name         = (mono_field_get_name_t) GetProcAddress(hMono, "mono_field_get_name");
    mono_runtime_invoke         = (mono_runtime_invoke_t) GetProcAddress(hMono, "mono_runtime_invoke");
    mono_field_get_value        = (mono_field_get_value_t) GetProcAddress(hMono, "mono_field_get_value");
    mono_field_set_value        = (mono_field_set_value_t) GetProcAddress(hMono, "mono_field_set_value");
    mono_field_static_get_value = (mono_field_static_get_value_t) GetProcAddress(hMono, "mono_field_static_get_value");
    mono_field_static_set_value = (mono_field_static_set_value_t) GetProcAddress(hMono, "mono_field_static_set_value");
    mono_field_get_parent       = (mono_field_get_parent_t) GetProcAddress(hMono, "mono_field_get_parent");
    mono_class_vtable           = (mono_class_vtable_t) GetProcAddress(hMono, "mono_class_vtable");
    mono_string_to_utf8         = (mono_string_to_utf8_t) GetProcAddress(hMono, "mono_string_to_utf8");
    mono_string_new             = (mono_string_new_t) GetProcAddress(hMono, "mono_string_new");
    mono_compile_method         = (mono_compile_method_t) GetProcAddress(hMono, "mono_compile_method");
    mono_object_unbox           = (mono_object_unbox_t) GetProcAddress(hMono, "mono_object_unbox");
    mono_object_get_class       = (mono_object_get_class_t) GetProcAddress(hMono, "mono_object_get_class");
    mono_object_new             = (mono_object_new_t) GetProcAddress(hMono, "mono_object_new");
    mono_class_get_type         = (mono_class_get_type_t) GetProcAddress(hMono, "mono_class_get_type");
    mono_type_get_class         = (mono_type_get_class_t) GetProcAddress(hMono, "mono_type_get_class");
    mono_type_get_object        = (mono_type_get_object_t) GetProcAddress(hMono, "mono_type_get_object");

    // Verify the critical functions were successfully loaded
    return mono_thread_attach && mono_get_root_domain && mono_class_from_name;
  }

  MonoDomain* GetActiveDomain()
  {
    MonoDomain* root        = mono_get_root_domain ? mono_get_root_domain() : nullptr;
    MonoDomain* childDomain = nullptr;

    if (mono_domain_foreach && root) {
      mono_domain_foreach(
        [](MonoDomain* dom, void* user_data) {
          MonoDomain** out = (MonoDomain**) user_data;
          if (dom && dom != mono_get_root_domain()) {
            *out = dom;
          }
        },
        &childDomain
      );
    }

    if (childDomain)
      return childDomain;

    if (mono_domain_get) {
      MonoDomain* cur = mono_domain_get();
      if (cur)
        return cur;
    }

    return root;
  }

  MonoDomain* GetRootDomain() { return GetActiveDomain(); }

  void EnsureThreadAttached()
  {
    MonoDomain* domain = GetActiveDomain();
    if (domain && mono_thread_attach) {
      mono_thread_attach(domain);
      if (mono_domain_set) {
        mono_domain_set(domain, 0);
      }
    }
  }

  MonoClass* GetClassFromName(const char* name_space, const char* class_name)
  {
    if (!mono_assembly_foreach || !mono_assembly_get_image || !mono_class_from_name) {
      return nullptr;
    }

    EnsureThreadAttached();

    MonoClass* klass = nullptr;
    struct SearchData
    {
      const char* nameSpace;
      const char* className;
      MonoClass*  foundClass;
    } searchData = {name_space, class_name, nullptr};

    mono_assembly_foreach(
      [](MonoAssembly* assem, void* user_data) {
        SearchData* data = (SearchData*) user_data;
        if (data->foundClass)
          return;
        MonoImage* img = mono_assembly_get_image(assem);
        if (img) {
          MonoClass* k = mono_class_from_name(img, data->nameSpace, data->className);
          if (k)
            data->foundClass = k;
        }
      },
      &searchData
    );
    return searchData.foundClass;
  }

  void* FindObjectOfType(const char* name_space, const char* class_name)
  {
    if (!mono_class_get_type || !mono_type_get_object || !mono_class_get_method_from_name || !mono_runtime_invoke)
      return nullptr;

    MonoDomain* domain = GetRootDomain();
    if (!domain)
      return nullptr;
    if (mono_thread_attach)
      mono_thread_attach(domain);

    MonoClass* klass = GetClassFromName(name_space, class_name);
    if (!klass)
      return nullptr;

    MonoType* typeObj = mono_class_get_type(klass);
    if (!typeObj)
      return nullptr;

    MonoObject* sysTypeObj = mono_type_get_object(domain, typeObj);
    if (!sysTypeObj)
      return nullptr;

    MonoClass* unityObjClass = GetClassFromName("UnityEngine", "Object");
    if (!unityObjClass)
      return nullptr;

    MonoMethod* findMethod = mono_class_get_method_from_name(unityObjClass, "FindObjectOfType", 1);
    if (!findMethod)
      return nullptr;

    void*       args[1] = {sysTypeObj};
    MonoObject* exc     = nullptr;
    void*       result  = mono_runtime_invoke(findMethod, nullptr, args, &exc);
    if (exc)
      return nullptr;
    return result;
  }

  void* CompileMethod(const char* name_space, const char* class_name, const char* method_name, int param_count)
  {
    MonoClass* klass = GetClassFromName(name_space, class_name);

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
        Logger::Log("[Mono] Found method %s but with different parameter count! Compiling anyway.", method_name);
        method = any_method;
      }
      else {
        return nullptr;
      }
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

  MonoField* GetFieldFromName(const char* name_space, const char* class_name, const char* field_name)
  {
    if (
      !mono_get_root_domain || !mono_domain_assembly_open || !mono_assembly_get_image || !mono_class_get_field_from_name
    )
      return nullptr;

    MonoDomain* domain = mono_get_root_domain();
    if (!domain)
      return nullptr;

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
      if (!assembly)
        assembly = mono_domain_assembly_open(domain, "Assembly-CSharp");
    }

    if (!assembly)
      return nullptr;

    MonoImage* image = mono_assembly_get_image(assembly);
    if (!image)
      return nullptr;

    MonoClass* klass = mono_class_from_name(image, name_space, class_name);
    if (!klass)
      return nullptr;

    return mono_class_get_field_from_name(klass, field_name);
  }

  void GetFieldValue(MonoObject* obj, MonoField* field, void* value)
  {
    if (mono_field_get_value && obj && field) {
      mono_field_get_value(obj, field, value);
    }
  }

  void SetFieldValue(MonoObject* obj, MonoField* field, void* value)
  {
    if (mono_field_set_value && obj && field) {
      mono_field_set_value(obj, field, value);
    }
  }

  void GetStaticFieldValue(MonoField* field, void* value)
  {
    if (!mono_field_static_get_value || !field || !value)
      return;

    EnsureThreadAttached();

    MonoDomain* domain = GetActiveDomain();
    if (!domain)
      return;

    MonoClass* klass = mono_field_get_parent ? mono_field_get_parent(field) : nullptr;
    if (!klass)
      return;

    MonoVTable* vt = mono_class_vtable ? mono_class_vtable(domain, klass) : nullptr;
    if (!vt)
      return;

    mono_field_static_get_value(vt, field, value);
  }

  static void ResolveListFields(MonoClass* klass, MonoField*& outItems, MonoField*& outSize)
  {
    outItems = mono_class_get_field_from_name ? mono_class_get_field_from_name(klass, "_items") : nullptr;
    outSize  = mono_class_get_field_from_name ? mono_class_get_field_from_name(klass, "_size") : nullptr;
    if ((!outItems || !outSize) && mono_class_get_fields && mono_field_get_name) {
      void*      iter = nullptr;
      MonoField* f    = nullptr;
      while ((f = mono_class_get_fields(klass, &iter)) != nullptr) {
        const char* fname = mono_field_get_name(f);
        if (fname) {
          if (!outItems && strcmp(fname, "_items") == 0)
            outItems = f;
          if (!outSize && strcmp(fname, "_size") == 0)
            outSize = f;
        }
      }
    }
  }

  bool GetListElements(void* listObj, std::vector<void*>& outElements)
  {
    outElements.clear();
    if (!listObj || !mono_object_get_class)
      return false;

    MonoClass* klass = mono_object_get_class((MonoObject*) listObj);
    if (!klass)
      return false;

    MonoField* itemsField = nullptr;
    MonoField* sizeField  = nullptr;
    ResolveListFields(klass, itemsField, sizeField);
    if (!itemsField || !sizeField)
      return false;

    int size = 0;
    GetFieldValue((MonoObject*) listObj, sizeField, &size);
    if (size <= 0)
      return true;

    void* arrayObj = nullptr;
    GetFieldValue((MonoObject*) listObj, itemsField, &arrayObj);
    if (!arrayObj)
      return false;

    // In 64-bit Mono, elements start at byte offset 32 (0x20)
    void** elements = (void**) ((char*) arrayObj + 32);
    outElements.reserve(size);
    for (int i = 0; i < size; ++i) {
      if (elements[i]) {
        outElements.push_back(elements[i]);
      }
    }
    return true;
  }

  void* GetListItem(void* listObj, int index)
  {
    if (!listObj || index < 0 || !mono_object_get_class)
      return nullptr;

    MonoClass* klass = mono_object_get_class((MonoObject*) listObj);
    if (!klass)
      return nullptr;

    MonoField* itemsField = nullptr;
    MonoField* sizeField  = nullptr;
    ResolveListFields(klass, itemsField, sizeField);
    if (!sizeField || !itemsField)
      return nullptr;

    int size = 0;
    GetFieldValue((MonoObject*) listObj, sizeField, &size);
    if (index >= size)
      return nullptr;

    void* arrayObj = nullptr;
    GetFieldValue((MonoObject*) listObj, itemsField, &arrayObj);
    if (!arrayObj)
      return nullptr;

    void** elements = (void**) ((char*) arrayObj + 32);
    return elements[index];
  }

  int GetListCount(void* listObj)
  {
    if (!listObj || !mono_object_get_class)
      return 0;

    MonoClass* klass = mono_object_get_class((MonoObject*) listObj);
    if (!klass)
      return 0;

    MonoField* itemsField = nullptr;
    MonoField* sizeField  = nullptr;
    ResolveListFields(klass, itemsField, sizeField);
    if (!sizeField)
      return 0;

    int size = 0;
    GetFieldValue((MonoObject*) listObj, sizeField, &size);
    return size;
  }

  void* GetLocalPlayer()
  {
    EnsureThreadAttached();

    // 1. Try static method PlayersController.RandomPlayer()
    static MonoClass*  pcClass     = nullptr;
    static MonoMethod* mRandom     = nullptr;
    static bool        triedRandom = false;
    if (!triedRandom) {
      triedRandom = true;
      pcClass     = GetClassFromName("", "PlayersController");
      if (pcClass && mono_class_get_method_from_name) {
        mRandom = mono_class_get_method_from_name(pcClass, "RandomPlayer", 0);
      }
    }
    if (mRandom && mono_runtime_invoke) {
      MonoObject* exc = nullptr;
      MonoObject* p   = (MonoObject*) mono_runtime_invoke(mRandom, nullptr, nullptr, &exc);
      if (!exc && p) {
        static bool loggedPlayer = false;
        if (!loggedPlayer) {
          Logger::Log("[Mono] GetLocalPlayer: resolved player via PlayersController.RandomPlayer(): %p", p);
          loggedPlayer = true;
        }
        return p;
      }
    }

    // 2. Try PlayersController.instance.players[0]
    static MonoField* playersCtrlInstanceField = nullptr;
    static MonoField* playersListField         = nullptr;
    if (!playersCtrlInstanceField) {
      playersCtrlInstanceField = GetFieldFromName("", "PlayersController", "instance");
      playersListField         = GetFieldFromName("", "PlayersController", "players");
    }
    if (playersCtrlInstanceField && playersListField) {
      void* playersCtrl = nullptr;
      GetStaticFieldValue(playersCtrlInstanceField, &playersCtrl);
      if (playersCtrl) {
        void* playersList = nullptr;
        GetFieldValue((MonoObject*) playersCtrl, playersListField, &playersList);
        if (playersList) {
          void* p = GetListItem(playersList, 0);
          if (p) {
            static bool loggedPlayer = false;
            if (!loggedPlayer) {
              Logger::Log("[Mono] GetLocalPlayer: resolved player via players list: %p", p);
              loggedPlayer = true;
            }
            return p;
          }
        }
      }
    }

    // 3. Fallback to Unity Object.FindObjectOfType("PlayerMain")
    void* p = FindObjectOfType("", "PlayerMain");
    if (p) {
      static bool loggedPlayer = false;
      if (!loggedPlayer) {
        Logger::Log("[Mono] GetLocalPlayer: resolved player via FindObjectOfType: %p", p);
        loggedPlayer = true;
      }
    }
    return p;
  }

  typedef MonoMethod*(__cdecl* mono_class_get_methods_t)(MonoClass* klass, void** iter);
  typedef const char*(__cdecl* mono_method_get_name_t)(MonoMethod* method);

  void DumpMethods(const char* name_space, const char* class_name)
  {
    HMODULE hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
    if (!hMono)
      return;

    auto class_get_methods = (mono_class_get_methods_t) GetProcAddress(hMono, "mono_class_get_methods");
    auto method_get_name   = (mono_method_get_name_t) GetProcAddress(hMono, "mono_method_get_name");

    if (!class_get_methods || !method_get_name)
      return;

    MonoDomain* domain = mono_get_root_domain();
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
      if (!assembly)
        assembly = mono_domain_assembly_open(domain, "Assembly-CSharp");
    }

    if (!assembly)
      return;

    MonoImage* image = mono_assembly_get_image(assembly);
    MonoClass* klass = mono_class_from_name(image, name_space, class_name);
    if (!klass)
      return;

    Logger::Log("[Mono] Methods for %s::%s:", name_space, class_name);
    void*       iter   = nullptr;
    MonoMethod* method = nullptr;
    while ((method = class_get_methods(klass, &iter)) != nullptr) {
      Logger::Log(" - %s", method_get_name(method));
    }
  }
  typedef MonoField*(__cdecl* mono_class_get_fields_t)(MonoClass* klass, void** iter);
  typedef const char*(__cdecl* mono_field_get_name_t)(MonoField* field);

  void DumpFields(const char* name_space, const char* class_name)
  {
    HMODULE hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
    if (!hMono)
      return;

    auto class_get_fields = (mono_class_get_fields_t) GetProcAddress(hMono, "mono_class_get_fields");
    auto field_get_name   = (mono_field_get_name_t) GetProcAddress(hMono, "mono_field_get_name");

    if (!class_get_fields || !field_get_name)
      return;

    MonoDomain* domain = mono_get_root_domain();
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
      if (!assembly)
        assembly = mono_domain_assembly_open(domain, "Assembly-CSharp");
    }

    if (!assembly)
      return;

    MonoImage* image = mono_assembly_get_image(assembly);
    MonoClass* klass = mono_class_from_name(image, name_space, class_name);
    if (!klass)
      return;

    Logger::Log("[Mono] Fields for %s::%s:", name_space, class_name);
    void*      iter  = nullptr;
    MonoField* field = nullptr;
    while ((field = class_get_fields(klass, &iter)) != nullptr) {
      Logger::Log(" - %s", field_get_name(field));
    }
  }
}  // namespace Mono
