#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "singleton.h"
#include "stdafx.h"

#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")

class MainWindow
{
private:

public:
    HWND hwnd = nullptr;
    ID3D11Device *g_pd3dDevice = nullptr;
    ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
    IDXGISwapChain *g_pSwapChain = nullptr;
    ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;

    bool Init();
    bool InitD3D(HWND hWnd);
    void CleanupD3D();

    MainWindow(/* args */);
    ~MainWindow() = default;
};

class MainWindowSingleton : public Singleton<MainWindow, true>
{
};

#endif