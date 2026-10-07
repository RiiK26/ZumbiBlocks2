#include "hooks.hpp"
#include "config.hpp"
#include "menu.hpp"
#include "../utils/logger.hpp"

#include <windows.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_dx12.h>
#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <MinHook.h>
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace UI
{
  static std::string g_IniPath;

  // DXGI Function Typedefs
  typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
  typedef HRESULT(__stdcall* Present1_t)(
    IDXGISwapChain1* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters
  );
  typedef HRESULT(__stdcall* ResizeBuffers_t)(
    IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags
  );
  typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
  typedef void(__stdcall* ExecuteCommandLists_t)(
    ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists
  );

  // Globals / Pointers
  Present_t                  oPresent                       = nullptr;
  Present1_t                 oPresent1                      = nullptr;
  ResizeBuffers_t            oResizeBuffers                 = nullptr;
  ExecuteCommandLists_t      oExecuteCommandLists           = nullptr;
  WNDPROC                    oWndProc                       = nullptr;

  HWND                       window                         = nullptr;
  bool                       init                           = false;

  // DX11
  ID3D11Device*              pDevice                        = nullptr;
  ID3D11DeviceContext*       pContext                       = nullptr;
  ID3D11RenderTargetView*    mainRenderTargetView           = nullptr;

  // DX12
  bool                       initDX12                       = false;
  ID3D12Device*              g_pd3dDevice                   = nullptr;
  ID3D12DescriptorHeap*      g_pd3dRtvDescHeap              = nullptr;
  ID3D12DescriptorHeap*      g_pd3dSrvDescHeap              = nullptr;
  ID3D12CommandQueue*        g_pd3dCommandQueue             = nullptr;
  ID3D12GraphicsCommandList* g_pd3dCommandList              = nullptr;
  ID3D12CommandAllocator*    g_commandAllocators[16]        = {};
  ID3D12Resource*            g_mainRenderTargetResource[16] = {};
  UINT                       g_NUM_FRAMES_IN_FLIGHT         = 16;

  // Render Target Management
  void CleanupRenderTargetDX12()
  {
    for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++) {
      if (g_mainRenderTargetResource[i]) {
        g_mainRenderTargetResource[i]->Release();
        g_mainRenderTargetResource[i] = nullptr;
      }
    }
  }

  void CreateRenderTargetDX12(IDXGISwapChain* pSwapChain, ID3D12Device* pDevice)
  {
    for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++) {
      ID3D12Resource* pBackBuffer = nullptr;
      pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**) &pBackBuffer);
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
      rtvHandle.ptr += i * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      pDevice->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
      g_mainRenderTargetResource[i] = pBackBuffer;
    }
  }

  void CleanupRenderTarget()
  {
    if (mainRenderTargetView) {
      mainRenderTargetView->Release();
      mainRenderTargetView = nullptr;
    }
  }

  void CreateRenderTarget(IDXGISwapChain* pSwapChain)
  {
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &pBackBuffer);
    pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
    pBackBuffer->Release();
  }

  // Window Procedure Hook
  static bool IsMouseMessage(UINT uMsg)
  {
    return uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN
        || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL || uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP;
  }

  static bool IsKeyboardMessage(UINT uMsg)
  {
    return uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_CHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;
  }

  LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
      Config.bMenuOpen = !Config.bMenuOpen;
      return 1;
    }

    if (Config.bMenuOpen) {
      ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
      ImGuiIO& io = ImGui::GetIO();
      if (io.WantCaptureMouse && IsMouseMessage(uMsg))
        return 1;
      if (io.WantCaptureKeyboard && IsKeyboardMessage(uMsg))
        return 1;
    }
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
  }

  // Hook Implementations
  HRESULT __stdcall hkResizeBuffers(
    IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags
  )
  {
    CleanupRenderTarget();
    if (initDX12)
      CleanupRenderTargetDX12();

    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (pDevice)
      CreateRenderTarget(pSwapChain);
    if (initDX12 && g_pd3dDevice)
      CreateRenderTargetDX12(pSwapChain, g_pd3dDevice);

    return hr;
  }

  bool GetSwapChainDesc(IDXGISwapChain* pSwapChain, DXGI_SWAP_CHAIN_DESC& sd)
  {
    ZeroMemory(&sd, sizeof(sd));
    if (SUCCEEDED(pSwapChain->GetDesc(&sd)))
      return true;

    IDXGISwapChain1* pSwapChain1 = nullptr;
    if (SUCCEEDED(pSwapChain->QueryInterface(__uuidof(IDXGISwapChain1), (void**) &pSwapChain1))) {
      DXGI_SWAP_CHAIN_DESC1 sd1;
      if (SUCCEEDED(pSwapChain1->GetDesc1(&sd1))) {
        sd.BufferCount       = sd1.BufferCount;
        sd.BufferDesc.Format = sd1.Format;
        sd.BufferDesc.Width  = sd1.Width;
        sd.BufferDesc.Height = sd1.Height;
        sd.SampleDesc        = sd1.SampleDesc;
        sd.BufferUsage       = sd1.BufferUsage;
        sd.SwapEffect        = sd1.SwapEffect;
        sd.Flags             = sd1.Flags;
        pSwapChain1->GetHwnd(&sd.OutputWindow);
        pSwapChain1->Release();
        return true;
      }
      pSwapChain1->Release();
    }
    return false;
  }

  void RenderImGui_DX11(IDXGISwapChain* pSwapChain, ID3D11Device* pDeviceArg)
  {
    if (!init) {
      Logger::Log("[DX11] Initializing ImGui DX11...");
      pDevice = pDeviceArg;
      if (pDevice) {
        pDevice->GetImmediateContext(&pContext);
        DXGI_SWAP_CHAIN_DESC sd;
        if (GetSwapChainDesc(pSwapChain, sd)) {
          window = sd.OutputWindow;
          Logger::Log("[DX11] Output window found: %p", window);
        }
        else {
          Logger::Log("[DX11] ERROR: GetSwapChainDesc failed!");
        }

        CreateRenderTarget(pSwapChain);
        if (mainRenderTargetView) {
          Logger::Log("[DX11] mainRenderTargetView created successfully.");
        }
        else {
          Logger::Log("[DX11] ERROR: Failed to create mainRenderTargetView.");
        }

        oWndProc = (WNDPROC) SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) WndProc);
        if (oWndProc) {
          Logger::Log("[DX11] WndProc hooked.");
        }
        else {
          Logger::Log("[DX11] ERROR: Failed to hook WndProc. (Error: %lu)", GetLastError());
        }

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        std::string dir = GetModuleDirectory();
        if (!dir.empty()) {
          g_IniPath      = dir + "imgui.ini";
          io.IniFilename = g_IniPath.c_str();
        }

        ImGui_ImplWin32_Init(window);
        ImGui_ImplDX11_Init(pDevice, pContext);
        ImGui::StyleColorsDark();

        init = true;
        Logger::Log("[DX11] ImGui DX11 initialized successfully!");
      }
      else {
        Logger::Log("[DX11] ERROR: pDevice is null!");
      }
    }

    if (init && mainRenderTargetView) {
      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      DrawMenuUI();  // Calls drawing logic from menu.cpp

      ImGui::Render();
      pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
  }

  void RenderImGui_DX12(IDXGISwapChain* pSwapChain, ID3D12Device* pDevice)
  {
    if (!g_pd3dCommandQueue)
      return;

    if (!initDX12) {
      g_pd3dDevice = pDevice;
      DXGI_SWAP_CHAIN_DESC sd;
      GetSwapChainDesc(pSwapChain, sd);
      window                 = sd.OutputWindow;
      oWndProc               = (WNDPROC) SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) WndProc);

      g_NUM_FRAMES_IN_FLIGHT = sd.BufferCount;
      if (g_NUM_FRAMES_IN_FLIGHT > 16)
        g_NUM_FRAMES_IN_FLIGHT = 16;

      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.NumDescriptors             = g_NUM_FRAMES_IN_FLIGHT;
      desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      desc.NodeMask                   = 1;
      if (pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
        return;

      D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
      srvdesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      srvdesc.NumDescriptors             = 1;
      srvdesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      if (pDevice->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
        return;

      for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++) {
        if (
          pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i])) != S_OK
        )
          return;
      }

      if (
        pDevice->CreateCommandList(
          0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList)
        )
        != S_OK
      )
        return;
      g_pd3dCommandList->Close();

      CreateRenderTargetDX12(pSwapChain, pDevice);

      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

      std::string dir = GetModuleDirectory();
      if (!dir.empty()) {
        g_IniPath      = dir + "imgui.ini";
        io.IniFilename = g_IniPath.c_str();
      }

      ImGui_ImplWin32_Init(window);
      ImGui_ImplDX12_InitInfo info      = {};
      info.Device                       = pDevice;
      info.CommandQueue                 = g_pd3dCommandQueue;
      info.NumFramesInFlight            = g_NUM_FRAMES_IN_FLIGHT;
      info.RTVFormat                    = sd.BufferDesc.Format;
      info.DSVFormat                    = DXGI_FORMAT_UNKNOWN;
      info.SrvDescriptorHeap            = g_pd3dSrvDescHeap;
      info.LegacySingleSrvCpuDescriptor = g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
      info.LegacySingleSrvGpuDescriptor = g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
      ImGui_ImplDX12_Init(&info);

      ImGui::StyleColorsDark();
      initDX12 = true;
    }

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawMenuUI();  // Calls drawing logic from menu.cpp

    ImGui::Render();

    static UINT      manualBackBufferIdx = 0;
    IDXGISwapChain3* pSwapChain3         = nullptr;
    UINT             backBufferIdx       = 0;
    if (SUCCEEDED(pSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**) &pSwapChain3))) {
      backBufferIdx = pSwapChain3->GetCurrentBackBufferIndex();
      pSwapChain3->Release();
    }
    else {
      backBufferIdx       = manualBackBufferIdx;
      manualBackBufferIdx = (manualBackBufferIdx + 1) % g_NUM_FRAMES_IN_FLIGHT;
    }

    ID3D12CommandAllocator* commandAllocator = g_commandAllocators[backBufferIdx];
    commandAllocator->Reset();
    g_pd3dCommandList->Reset(commandAllocator, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIdx * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    g_pd3dCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    ID3D12CommandList* ppCommandLists[] = {g_pd3dCommandList};
    g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  }

  void __stdcall
  hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
  {
    static bool loggedExecute = false;
    if (!loggedExecute) {
      Logger::Log("[DX12] hkExecuteCommandLists called for the first time.");
      loggedExecute = true;
    }

    if (!g_pd3dCommandQueue && queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
      Logger::Log("[DX12] Discovered Direct Command Queue in hkExecuteCommandLists!");
      g_pd3dCommandQueue = queue;
    }
    return oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
  }

  void RenderFrame(IDXGISwapChain* pSwapChain)
  {
    ID3D11Device* pD11 = nullptr;
    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**) &pD11))) {
      static bool logD11 = false;
      if (!logD11) {
        Logger::Log("[DX] RenderFrame: Discovered DX11 Device!");
        logD11 = true;
      }
      RenderImGui_DX11(pSwapChain, pD11);
      pD11->Release();
      return;
    }

    ID3D12Device* pD12 = nullptr;
    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**) &pD12))) {
      static bool logD12 = false;
      if (!logD12) {
        Logger::Log("[DX] RenderFrame: Discovered DX12 Device!");
        logD12 = true;
      }

      if (!g_pd3dCommandQueue) {
        static bool logNoQ = false;
        if (!logNoQ) {
          Logger::Log("[DX12] ERROR: g_pd3dCommandQueue is null. Waiting for ExecuteCommandLists...");
          logNoQ = true;
        }
      }

      RenderImGui_DX12(pSwapChain, pD12);
      pD12->Release();
      return;
    }

    ID3D12CommandQueue* pD12CommandQueue = nullptr;
    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12CommandQueue), (void**) &pD12CommandQueue))) {
      static bool logD12CQ = false;
      if (!logD12CQ) {
        Logger::Log("[DX] RenderFrame: Discovered DX12 CommandQueue from SwapChain!");
        logD12CQ = true;
      }

      if (SUCCEEDED(pD12CommandQueue->GetDevice(__uuidof(ID3D12Device), (void**) &pD12))) {
        static bool logD12Dev = false;
        if (!logD12Dev) {
          Logger::Log("[DX] RenderFrame: Successfully extracted ID3D12Device from CommandQueue!");
          logD12Dev = true;
        }
        g_pd3dCommandQueue = pD12CommandQueue;
        RenderImGui_DX12(pSwapChain, pD12);
        pD12->Release();
      }
      else {
        static bool logFail = false;
        if (!logFail) {
          Logger::Log("[DX] RenderFrame: FAILED to extract ID3D12Device from CommandQueue!");
          logFail = true;
        }
      }
      pD12CommandQueue->Release();
    }
    else {
      static bool logNone = false;
      if (!logNone) {
        Logger::Log("[DX] RenderFrame: SwapChain->GetDevice failed for D11, D12, and D12CommandQueue!");
        logNone = true;
      }
    }
  }

  HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
  {
    static bool loggedPresent = false;
    if (!loggedPresent) {
      Logger::Log("[DX] hkPresent called for the first time.");
      loggedPresent = true;
    }
    if ((Flags & DXGI_PRESENT_TEST) == 0)
      RenderFrame(pSwapChain);
    return oPresent(pSwapChain, SyncInterval, Flags);
  }

  HRESULT __stdcall hkPresent1(
    IDXGISwapChain1* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters
  )
  {
    static bool loggedPresent1 = false;
    if (!loggedPresent1) {
      Logger::Log("[DX] hkPresent1 called for the first time.");
      loggedPresent1 = true;
    }
    if ((PresentFlags & DXGI_PRESENT_TEST) == 0)
      RenderFrame(pSwapChain);
    return oPresent1(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
  }

  void Initialize()
  {
    Config.LoadConfig();

    WNDCLASSEXA wc = {sizeof(WNDCLASSEXA),    CS_CLASSDC, DefWindowProcA, 0L,   0L,
                      GetModuleHandleA(NULL), NULL,       NULL,           NULL, NULL,
                      "DummyClass",           NULL};
    RegisterClassExA(&wc);
    HWND dummyWindow =
      CreateWindowA("DummyClass", "", WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount                       = 1;
    sd.BufferDesc.Format                 = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage                       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                      = dummyWindow;
    sd.SampleDesc.Count                  = 1;
    sd.Windowed                          = TRUE;
    sd.SwapEffect                        = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL    featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    IDXGISwapChain*      pDummySwapChain = nullptr;
    ID3D11Device*        pDummyDevice    = nullptr;
    ID3D11DeviceContext* pDummyContext   = nullptr;

    HRESULT              hr              = D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 3, D3D11_SDK_VERSION, &sd, &pDummySwapChain,
      &pDummyDevice, NULL, &pDummyContext
    );
    if (FAILED(hr)) {
      hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, featureLevels, 3, D3D11_SDK_VERSION, &sd, &pDummySwapChain, &pDummyDevice,
        NULL, &pDummyContext
      );
    }

    if (SUCCEEDED(hr) && pDummySwapChain) {
      void** pVTable  = *reinterpret_cast<void***>(pDummySwapChain);
      void*  pPresent = pVTable[8];

      if (MH_CreateHook(pPresent, (void*) hkPresent, (void**) &oPresent) == MH_OK) {
        MH_EnableHook(pPresent);
        Logger::Log("[DX] Successfully hooked Present (DXGI).");
      }
      else {
        Logger::Log("[DX] ERROR: Failed to hook Present.");
      }

      IDXGISwapChain1* pSwapChain1 = nullptr;
      if (SUCCEEDED(pDummySwapChain->QueryInterface(__uuidof(IDXGISwapChain1), (void**) &pSwapChain1))) {
        void** pVTable1  = *reinterpret_cast<void***>(pSwapChain1);
        void*  pPresent1 = pVTable1[22];
        if (MH_CreateHook(pPresent1, (void*) hkPresent1, (void**) &oPresent1) == MH_OK)
          MH_EnableHook(pPresent1);
        pSwapChain1->Release();
      }

      void* pResizeBuffers = pVTable[13];
      if (MH_CreateHook(pResizeBuffers, (void*) hkResizeBuffers, (void**) &oResizeBuffers) == MH_OK) {
        MH_EnableHook(pResizeBuffers);
        Logger::Log("[DX] Successfully hooked ResizeBuffers.");
      }
      else {
        Logger::Log("[DX] ERROR: Failed to hook ResizeBuffers.");
      }

      pDummySwapChain->Release();
      pDummyDevice->Release();
      pDummyContext->Release();
    }

    ID3D12Device* pDummyD12Device = nullptr;
    if (SUCCEEDED(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**) &pDummyD12Device))) {
      D3D12_COMMAND_QUEUE_DESC queueDesc = {};
      queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

      ID3D12CommandQueue* pDummyQueue    = nullptr;
      if (
        SUCCEEDED(pDummyD12Device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**) &pDummyQueue))
      ) {
        void** pQueueVTable         = *reinterpret_cast<void***>(pDummyQueue);
        void*  pExecuteCommandLists = pQueueVTable[10];

        if (
          MH_CreateHook(pExecuteCommandLists, (void*) hkExecuteCommandLists, (void**) &oExecuteCommandLists) == MH_OK
        ) {
          MH_EnableHook(pExecuteCommandLists);
          Logger::Log("[DX12] Successfully hooked ExecuteCommandLists.");
        }
        else {
          Logger::Log("[DX12] ERROR: Failed to hook ExecuteCommandLists.");
        }
        pDummyQueue->Release();
      }
      pDummyD12Device->Release();
    }

    DestroyWindow(dummyWindow);
    UnregisterClassA("DummyClass", wc.hInstance);
  }

  void Shutdown()
  {
    if (window && oWndProc) {
      SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) oWndProc);
      oWndProc = nullptr;
    }

    MH_DisableHook(MH_ALL_HOOKS);

    if (init || initDX12) {
      if (initDX12)
        ImGui_ImplDX12_Shutdown();
      else
        ImGui_ImplDX11_Shutdown();

      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
    }

    CleanupRenderTarget();
    CleanupRenderTargetDX12();

    if (g_pd3dCommandList) {
      g_pd3dCommandList->Release();
      g_pd3dCommandList = nullptr;
    }
    if (g_pd3dRtvDescHeap) {
      g_pd3dRtvDescHeap->Release();
      g_pd3dRtvDescHeap = nullptr;
    }
    if (g_pd3dSrvDescHeap) {
      g_pd3dSrvDescHeap->Release();
      g_pd3dSrvDescHeap = nullptr;
    }

    for (UINT i = 0; i < 16; i++) {
      if (g_commandAllocators[i]) {
        g_commandAllocators[i]->Release();
        g_commandAllocators[i] = nullptr;
      }
    }

    init     = false;
    initDX12 = false;
  }
}  // namespace UI
