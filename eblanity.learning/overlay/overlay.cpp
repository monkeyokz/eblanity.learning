#include "overlay.h"
#include "timer/timer.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace overlay {
HWND g_hwnd{};
WNDCLASSEXW g_wc{};
std::wstring g_windowName{};
int g_width{};
int g_height{};
Microsoft::WRL::ComPtr<ID3D11Device> g_device;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_deviceContext;
Microsoft::WRL::ComPtr<IDXGISwapChain1> g_swapChain;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_renderTargetView;
bool g_running{};
bool g_initialized{};
bool g_isOpen{ true };
std::function<void()> g_onRender{};

std::wstring GenerateRandomName(std::size_t length) {
    static constexpr wchar_t chars[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, (sizeof(chars) / sizeof(wchar_t)) - 2);
    std::wstring result(length, L' ');
    for (auto& c : result) c = chars[dist(rng)];
    return result;
}

bool CreateWindowInternal(HINSTANCE instance, void* param) {
    g_windowName = GenerateRandomName();
    g_wc = {};
    g_wc.cbSize = sizeof(g_wc);
    g_wc.style = CS_HREDRAW | CS_VREDRAW;
    g_wc.lpfnWndProc = WndProc;
    g_wc.hInstance = instance;
    g_wc.lpszClassName = g_windowName.c_str();
    if (!RegisterClassExW(&g_wc)) return false;
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_width = (std::min)(800, screenWidth);
    g_height = (std::min)(600, screenHeight);
    const int x = (screenWidth - g_width) / 2;
    const int y = (screenHeight - g_height) / 2;
    g_hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TOOLWINDOW, g_windowName.c_str(), g_windowName.c_str(), WS_POPUP, x, y, g_width, g_height, nullptr, nullptr, instance, param);
    if (!g_hwnd) { UnregisterClassW(g_windowName.c_str(), instance); return false; }
    MARGINS margins{-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(g_hwnd, &margins);
    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    if (!SetWindowDisplayAffinity(g_hwnd, WDA_EXCLUDEFROMCAPTURE)) {
        std::wcerr << L"SetWindowDisplayAffinity failed: " << GetLastError() << std::endl;
    }
    return true;
}

void DestroyWindowInternal() {
    if (g_hwnd) DestroyWindow(g_hwnd);
    g_hwnd = nullptr;
    if (g_wc.hInstance) UnregisterClassW(g_windowName.c_str(), g_wc.hInstance);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;
    if (msg == WM_SIZE && g_device && wParam != SIZE_MINIMIZED) ResizeRenderTarget();
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool CreateDeviceAndSwapChain(HWND hwnd) {
    constexpr D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL level{};
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION, g_device.GetAddressOf(), &level, g_deviceContext.GetAddressOf()))) return false;
    Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    if (FAILED(g_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice))) || FAILED(dxgiDevice->GetAdapter(&adapter)) || FAILED(adapter->GetParent(IID_PPV_ARGS(&factory)))) return false;
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    HRESULT hr = factory->CreateSwapChainForHwnd(g_device.Get(), hwnd, &desc, nullptr, nullptr, g_swapChain.GetAddressOf());
    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    return SUCCEEDED(hr);
}

bool CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    return g_swapChain && SUCCEEDED(g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))) && SUCCEEDED(g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, g_renderTargetView.GetAddressOf()));
}
void CleanupRenderTarget() { g_renderTargetView.Reset(); }
void ResizeRenderTarget() { CleanupRenderTarget(); if (g_swapChain) { g_swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0); CreateRenderTarget(); } }
bool RendererInitialize(HWND hwnd) { return CreateDeviceAndSwapChain(hwnd) && CreateRenderTarget(); }
void RendererShutdown() { CleanupRenderTarget(); g_swapChain.Reset(); g_deviceContext.Reset(); g_device.Reset(); }
void RendererPresent() { if (g_swapChain) g_swapChain->Present(0, 0); }

bool ImGuiInitialize(HWND hwnd) {
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::GetIO().IniFilename = nullptr; ImGui::StyleColorsDark();
    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(g_device.Get(), g_deviceContext.Get())) { ImGui_ImplWin32_Shutdown(); return false; }
    return true;
}
void ImGuiShutdown() { ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext(); }
void ImGuiBeginFrame() { ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame(); }
void ImGuiEndFrame() { ImGui::Render(); auto* rtv = g_renderTargetView.Get(); g_deviceContext->OMSetRenderTargets(1, &rtv, nullptr); constexpr float color[4]{}; g_deviceContext->ClearRenderTargetView(rtv, color); ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); }
void SetInputEnabled(bool enabled) { LONG_PTR style = GetWindowLongPtrW(g_hwnd, GWL_EXSTYLE); SetWindowLongPtrW(g_hwnd, GWL_EXSTYLE, enabled ? style & ~WS_EX_TRANSPARENT & ~WS_EX_NOACTIVATE : style | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE); SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED); if (enabled) { SetForegroundWindow(g_hwnd); SetFocus(g_hwnd); } }

bool OverlayInitialize(HINSTANCE instance) { if (g_initialized) return true; if (!CreateWindowInternal(instance)) return false; if (!RendererInitialize(g_hwnd) || !ImGuiInitialize(g_hwnd)) { RendererShutdown(); DestroyWindowInternal(); return false; } g_initialized = true; SetInputEnabled(g_isOpen); return true; }
void OverlayShutdown() { if (!g_initialized) return; g_running = false; g_isOpen = false; ImGuiShutdown(); RendererShutdown(); DestroyWindowInternal(); g_initialized = false; }
void OverlayRun() { g_running = true; MSG msg{}; timeBeginPeriod(1); while (g_running) { while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); if (msg.message == WM_QUIT) { g_running = false; g_isOpen = false; } } static bool pressed{}; if (GetAsyncKeyState(VK_HOME) & 0x8000) { if (!pressed) { g_isOpen = !g_isOpen; SetInputEnabled(g_isOpen); pressed = true; } } else pressed = false; ImGuiBeginFrame(); if (g_onRender) g_onRender(); ImGuiEndFrame(); RendererPresent(); timer::PrecisionSleep(1.0); } timeEndPeriod(1); }
void OverlayWait() {}
void OverlayRequestExit() { g_running = false; }
bool OverlayIsRunning() { return g_running; }
bool& OverlayIsOpen() { return g_isOpen; }
std::function<void()>& OverlayOnRender() { return g_onRender; }
HWND OverlayGetHwnd() { return g_hwnd; }
int OverlayGetWidth() { return g_width; }
int OverlayGetHeight() { return g_height; }
ID3D11Device* OverlayGetDevice() { return g_device.Get(); }
ID3D11DeviceContext* OverlayGetDeviceContext() { return g_deviceContext.Get(); }
ID3D11RenderTargetView* OverlayGetRenderTargetView() { return g_renderTargetView.Get(); }
}
