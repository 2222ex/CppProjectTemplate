#pragma once
#include "base/stdafx.h"
#include <Windows.h>
#include <wrl.h>

#include <WebView2.h>

using namespace Microsoft::WRL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void init_web(HINSTANCE hInstance, int nCmdShow)
{
    // 初始化COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        MessageBox(nullptr, "COM 初始化失败", "错误", MB_OK | MB_ICONERROR);
        return;
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "WebView2Sample";
    RegisterClass(&wc);

    // 创建窗口
    HWND hwnd = CreateWindow("WebView2Sample", "WebView2 Sample", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    // 初始化WebView2
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        nullptr,
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd](HRESULT result, ICoreWebView2Environment *env) -> HRESULT
            {
                if (FAILED(result) || !env)
                {
                    SPDLOG_LOGGER_ERROR(Logger::getLogger("test", true), "failed result: {}", result);
                    return result;
                }
                env->CreateCoreWebView2Controller(
                    hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT
                        {
                            if (controller)
                            {
                                controller->put_IsVisible(TRUE);

                                RECT bounds;
                                GetClientRect(hwnd, &bounds);
                                controller->put_Bounds(bounds);

                                ICoreWebView2 *webview;
                                controller->get_CoreWebView2(&webview);
                                webview->Navigate(L"http://www.bing.com");
                            }
                            return S_OK;
                        })
                        .Get());
                return S_OK;
            })
            .Get());

    ShowWindow(hwnd, nCmdShow);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
}