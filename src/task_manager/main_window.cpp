#include <httplib.h>

#include "injector.h"
#include "main_window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // Use ImGui::GetCurrentContext()

MainWindow::MainWindow(/* args */)
{
    tip_text = "Waiting";
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    auto &main_window = MainWindowSingleton::instance();
    switch (msg)
    {
    case WM_SIZE:
        if (main_window.g_pd3dDevice && wParam != SIZE_MINIMIZED)
        {
            main_window.g_mainRenderTargetView->Release();
            main_window.g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D *pBackBuffer;
            main_window.g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &pBackBuffer);
            main_window.g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &main_window.g_mainRenderTargetView);
            pBackBuffer->Release();
        }
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

bool MainWindow::InitD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevel;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext)))
    {
        return false;
    }

    ID3D11Texture2D *pBackBuffer;
    g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();

    return true;
}

void MainWindow::CleanupD3D()
{
    if (g_mainRenderTargetView)
        g_mainRenderTargetView->Release();
    if (g_pSwapChain)
        g_pSwapChain->Release();
    if (g_pd3dDeviceContext)
        g_pd3dDeviceContext->Release();
    if (g_pd3dDevice)
        g_pd3dDevice->Release();
}

bool MainWindow::Init()
{
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, "DX11 IMGUI", nullptr};
    RegisterClassEx(&wc);
    hwnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);

    if (!InitD3D(hwnd))
        return false;

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    ImGui::StyleColorsClassic();

    bool show_demo_window = true;
    float slider_value = 0.0f;
    MSG msg = {};

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    auto &injector = InjectorSingleton::instance();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // 开始 ImGui 帧
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Simple UI", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Hello, DirectX 11 + ImGui!");
        ImGui::Text(tip_text.c_str());

        static char buffer[256] = {0};

        ImGui::InputText("Process PID", buffer, sizeof(buffer));
        std::string str_buffer = buffer;
        if (!str_buffer.empty())
        {
            target_PID = std::stoul(str_buffer);
            // std::cout << "target_PID: " << target_PID << std::endl;
        }

        static char buf2[256] = {0};
        if (ImGui::InputText("Process Name", buf2, sizeof(buffer)))
        {
        }
        std::string str_buf2 = buf2;

        auto dll_path = std::filesystem::current_path() / "task_handler.dll";

        if (ImGui::Button("APC Inject"))
        {
            if (!str_buf2.empty())
            {
                target_PID = injector.FindProcessId(buf2);
            }
            injector.InjectQueueUserAPC(dll_path.wstring().c_str(), target_PID, tip_text);
        }

        ImGui::SameLine();

        if (ImGui::Button("CreateRemoteThread Inject"))
        {
            if (!str_buf2.empty())
            {
                target_PID = injector.FindProcessId(buf2);
            }
            injector.InjectUseCreateRemoteThread(dll_path.string().c_str(), target_PID, tip_text);
        }

        static std::string http_tip = "Waiting for call http";
        ImGui::Text(http_tip.c_str());

        static httplib::Client cli("localhost", 24960);

        if (ImGui::Button("f1"))
        {
            if (auto res = cli.Get("/f1"))
            {
                http_tip = res->body;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Detach"))
        {
            if (auto res = cli.Get("/Detach"))
            {
                http_tip = res->body;
                if (!str_buf2.empty())
                {
                    target_PID = injector.FindProcessId(buf2);
                }
            }
            injector.UnLoadLibrary(dll_path.string().c_str(), target_PID, tip_text);
        }

        if (ImGui::Button("DumpInventoryToConsole"))
        {
            if (auto res = cli.Get("/DumpInventoryToConsole"))
            {
                http_tip = res->body;
            }
        }

        if (ImGui::Button("GetInventoryCount"))
        {
            if (auto res = cli.Get("/GetInventoryCount"))
            {
                http_tip = res->body;
            }
        }

        if (ImGui::Button("MyGetItemVectorInfo"))
        {
            if (auto res = cli.Get("/MyGetItemVectorInfo"))
            {
                http_tip = res->body;
            }
        }

        ImGui::End();

        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);

        const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    // 清理资源
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return true;
}
