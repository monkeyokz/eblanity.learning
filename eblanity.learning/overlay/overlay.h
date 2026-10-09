#pragma once
#include "../include.h"

namespace overlay {
extern HWND g_hwnd;
extern WNDCLASSEXW g_wc;
extern std::wstring g_windowName;
extern int g_width;
extern int g_height;
extern Microsoft::WRL::ComPtr<ID3D11Device> g_device;
extern Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_deviceContext;
extern Microsoft::WRL::ComPtr<IDXGISwapChain1> g_swapChain;
extern Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_renderTargetView;
extern bool g_running;
extern bool g_initialized;
extern bool g_isOpen;
extern std::function<void()> g_onRender;
std::wstring GenerateRandomName(std::size_t length = 16);
bool CreateRenderTarget();
void CleanupRenderTarget();
void ResizeRenderTarget();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CreateWindowInternal(HINSTANCE hInstance, void* lpParam = nullptr);
void DestroyWindowInternal();
bool CreateDeviceAndSwapChain(HWND hwnd);
bool RendererInitialize(HWND hwnd);
void RendererShutdown();
void RendererPresent();
bool ImGuiInitialize(HWND hwnd);
void ImGuiShutdown();
void ImGuiBeginFrame();
void ImGuiEndFrame();
void SetInputEnabled(bool enabled);
bool OverlayInitialize(HINSTANCE hInstance);
void OverlayShutdown();
void OverlayRun();
void OverlayWait();
void OverlayRequestExit();
bool OverlayIsRunning();
bool& OverlayIsOpen();
std::function<void()>& OverlayOnRender();
HWND OverlayGetHwnd();
int OverlayGetWidth();
int OverlayGetHeight();
ID3D11Device* OverlayGetDevice();
ID3D11DeviceContext* OverlayGetDeviceContext();
ID3D11RenderTargetView* OverlayGetRenderTargetView();
}
