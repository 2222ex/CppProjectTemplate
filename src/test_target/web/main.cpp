////////////////////////////////////
// Choose impl:
//  - offscreen  = render to d3d11 texture (hwnd is source of input events)
//  - !offscreen = render to hwnd
////////////////////////////////////
bool OFFSCREEN_RENDERING = true;
////////////////////////////////////

#define IDC_BROWSER 101
#define IDI_BROWSER 102

#include "framework.h"
#include <Commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#pragma comment(lib, "Comctl32")
#include <dwmapi.h>
#pragma comment(lib, "dwmapi")
#include <uxtheme.h>
#pragma comment(lib, "uxtheme")
#define interface struct
#include <WebView2.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#pragma comment(lib, "dxguid")
#endif
#define safe_release(x) \
    if (x)              \
    {                   \
        x->Release();   \
        x = nullptr;    \
    }
#pragma comment(lib, "windowsapp")
#pragma comment(lib, "d3d11")
#include <DispatcherQueue.h>
#include <ShellScalingAPI.h>
#include <Unknwn.h>
#include <WinUser.h>
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>
#include <d3d11.h>
#include <gdiplus.h>
#include <objidl.h>
#include <shobjidl_core.h>
#include <windows.graphics.capture.interop.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.UI.h>
#include <winrt/base.h>
#include <winrt/windows.ui.composition.desktop.h>
#include <winrt/windows.ui.composition.h>
#include <winrt/windows.ui.h>
#include <wrl.h>
#pragma comment(lib, "Gdiplus.lib")

namespace winrt
{
using namespace std::literals;
using namespace Windows::System;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::UI::Composition;
}
namespace rt
{
using namespace winrt::Windows::Foundation;
using namespace ABI::Windows::Graphics::Capture;
}
namespace wrl
{
using namespace Microsoft::WRL;
}

#define MAX_LOADSTRING 100

class edge_browser;

Gdiplus::GdiplusStartupInput gdip_input_start {nullptr};
ULONG_PTR gdip_token {};
HINSTANCE hinst {nullptr};
ID3D11DeviceContext *context {nullptr};
IDXGISwapChain *swapchain {nullptr};
edge_browser *browser {nullptr};

struct edge_config
{
    HWND hwnd;
    BOOL offscreen;
    union
    {
        struct
        {
            ID3D11Device *device {nullptr};
        } edge_d3d11;

        struct
        {
        } edge_hwnd;
    };

    // add other properties...
};

class edge_browser :
    public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
    public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
    public ICoreWebView2WebMessageReceivedEventHandler,
    public ICoreWebView2ProcessFailedEventHandler,
    public ICoreWebView2ExecuteScriptCompletedHandler,
    public ICoreWebView2ScriptDialogOpeningEventHandler,
    public ICoreWebView2DocumentTitleChangedEventHandler,
    public ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler
{
private:
    void handle_failed_edge_load()
    {
        //"https://go.microsoft.com/fwlink/p/?LinkId=2124703" // direct download
        int result;
        if (SUCCEEDED(TaskDialog(NULL, hinst, L"Browser", 0, L"Edge runtime was not found. Do you want to download the runtime now?", TDCBF_YES_BUTTON | TDCBF_NO_BUTTON, 0, &result)) && IDYES == result)
            ShellExecuteA(0, NULL, "https://developer.microsoft.com/microsoft-edge/webview2", NULL, NULL, SW_SHOWDEFAULT);
        exit(0);
    }

public:
    edge_browser(edge_config *_edge) :
        edge(_edge)
    {

        // setup com and winrt for calling thread
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        winrt::init_apartment(winrt::apartment_type::single_threaded);

        if (edge->offscreen)
        {
            // a win32 app needs a DispatcherQueueController running to use winrt ui Compositor (to use Visual)
            DispatcherQueueOptions qcoptions {sizeof(DispatcherQueueOptions), DQTYPE_THREAD_CURRENT, DQTAT_COM_STA};
            winrt::check_hresult(CreateDispatcherQueueController(qcoptions, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(winrt::put_abi(queue_controller))));
        }

        // run edge api entry-point. we dont use any special setting so no need to call CreateCoreWebView2EnvironmentWithOptions
        if (FAILED(CreateCoreWebView2Environment(this)))
            handle_failed_edge_load();
    }

private:
    inline void release_resources()
    {
        if (controller)
            controller->Close();

        safe_release(comp);
        safe_release(webview);
        safe_release(settings);
        safe_release(controller);
        safe_release(env);

        if (m_session)
            m_session.Close();
        if (m_framePool)
            m_framePool.Close();
        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }

    ~edge_browser()
    {
        if (webview)
            webview->remove_WebMessageReceived(CoreWebView2WebMessageReceivedEventRegistrationToken);
        if (webview)
            webview->remove_ProcessFailed(CoreWebView2ProcessFailedEventRegistrationToken);
        if (webview)
            webview->remove_ScriptDialogOpening(CoreWebView2ScriptDialogOpeningEventRegistrationToken);
        if (webview)
            webview->remove_DocumentTitleChanged(CoreWebView2DocumentTitleChangedEventRegistrationToken);
        if (webview)
            webview->remove_FaviconChanged(CoreWebView2FaviconChangedEventRegistrationToken);
        if (webview)
            webview->remove_PermissionRequested(CoreWebView2PermissionRequestedEventRegistrationToken);
        if (comp)
            comp->remove_CursorChanged(CoreWebView2CursorChangedEventRegistrationToken);

        release_resources();

        if (queue_controller)
            queue_controller.ShutdownQueueAsync();

        winrt::uninit_apartment();
        CoUninitialize();
    }

public:
    bool translate_msg_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (!comp)
            return false;
        if (GetKeyState(VK_F12) & 0x8000)
        {
            webview->OpenDevToolsWindow();
            return true;
        }
        if (GetKeyState(VK_F11) & 0x8000)
        {
            webview->Print(print_settings, 0);
            return true;
        }
        if (GetKeyState(VK_F10) & 0x8000)
        {
            webview->OpenTaskManagerWindow();
            return true;
        }
        track_mouse();
        if (message != WM_MOUSEWHEEL)
            point = {LOWORD(lParam), HIWORD(lParam)};
        int flag = COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE;
        if (GetKeyState(VK_SHIFT) & 0x8000)
            flag |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000)
            flag |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_CONTROL;
#define VIRT_FLAG(x) (COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS)((last_button = x + set_capture_mouse(x != 0)) | flag)
        switch (message)
        {
        case WM_MOUSEMOVE:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE, VIRT_FLAG(last_button), 0, point);
            break;
        case WM_LBUTTONDOWN:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN, VIRT_FLAG(COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON), 0, point);
            break;
        case WM_LBUTTONUP:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP, VIRT_FLAG(COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE), 0, point);
            break;
        case WM_RBUTTONDOWN:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOWN, VIRT_FLAG(COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON), 0, point);
            break;
        case WM_RBUTTONUP:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP, VIRT_FLAG(COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE), 0, point);
            break;
        case WM_MBUTTONDOWN:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOWN, VIRT_FLAG(COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON), 0, point);
            break;
        case WM_MBUTTONUP:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP, VIRT_FLAG(COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE), 0, point);
            break;
        case WM_MOUSEWHEEL:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL, VIRT_FLAG(last_button), static_cast<unsigned>(GET_WHEEL_DELTA_WPARAM(wParam)), point);
            break;
        case WM_MOUSELEAVE:
            comp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_LEAVE, VIRT_FLAG(last_button), 0, point);
            tracking_on = false;
            break;
        case WM_SETCURSOR:
        {
            HCURSOR cur = 0;
            if (SUCCEEDED(comp->get_Cursor(&cur)) && cur)
                SetCursor(cur);
        }
        break;
        default:
            return false;
        }
        return true;
    }

    void resize(int w, int h)
    {
        if (controller && IsWindow(edge->hwnd))
        {
            if (w <= 0 || h <= 0)
                return;
            controller->put_Bounds({0, 0, w, h});
            update_browser_window();
            if (edge->offscreen && m_framePool)
                m_framePool.Recreate(m_device, m_pixelFormat, 2, {w, h});
            if (swapchain)
                swapchain->ResizeBuffers(2, w, h, DXGI_FORMAT_B8G8R8A8_UNORM, 0); // todo, this sould not be here...
        }
    }

    ID3D11Texture2D *texture()
    {
        if (!m_framePool)
            return nullptr;
        ID3D11Texture2D *tex = nullptr;
        auto frame = m_framePool.TryGetNextFrame();
        if (!frame)
            return nullptr;
        auto access = frame.Surface().as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
        if (!access)
            return nullptr;
        winrt::check_hresult(access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), (void **) &tex));
        return tex; // dont forget to release!
    }

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
    {
        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef(void) override
    {
        return InterlockedIncrement(&reference_count);
    }

    virtual ULONG STDMETHODCALLTYPE Release(void) override
    {
        auto mc = InterlockedDecrement(&reference_count);
        if (mc == 0)
            delete this;
        return mc;
    }

    // helper: notify repositioning
    void update_browser_window()
    {
        if (controller)
            controller->NotifyParentWindowPositionChanged();
    }

private:
    // ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler
    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Environment *_env) override
    {
        if (SUCCEEDED(errorCode))
        {
            _env->QueryInterface(&env);
            safe_release(_env);

            if (!edge->offscreen)
                env->CreateCoreWebView2Controller(edge->hwnd, this);
            else
                env->CreateCoreWebView2CompositionController(edge->hwnd, this);

            // env->CreateSharedBuffer(0xFFFF, &shared_buffer);
            env->CreatePrintSettings(&print_settings);
            print_settings->put_ShouldPrintHeaderAndFooter(FALSE);
        }
        else
        {
            handle_failed_edge_load();
        }
        return errorCode;
    }

    // ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler
    HRESULT __stdcall Invoke(HRESULT errorCode, ICoreWebView2CompositionController *_comp) override
    {
        if (FAILED(errorCode))
        {
            handle_failed_edge_load();
            return errorCode;
        }

        _comp->QueryInterface(&comp);

        if (edge->offscreen)
        {
            // setup d3d11 capture visual
            auto _compositor = winrt::Windows::UI::Composition::Compositor();
            auto compositor = _compositor.try_as<ABI::Windows::UI::Composition::ICompositor>();

            winrt::com_ptr<ABI::Windows::UI::Composition::IContainerVisual> root;
            winrt::check_hresult(compositor->CreateContainerVisual(root.put()));

            auto surface = root.try_as<ABI::Windows::UI::Composition::IVisual>();
            assert(surface);

            surface->put_Size({1, 1});
            winrt::check_hresult(surface->put_IsVisible(true));

            winrt::com_ptr<ABI::Windows::UI::Composition::IVisual> webview_visual;
            winrt::check_hresult(compositor->CreateContainerVisual(reinterpret_cast<ABI::Windows::UI::Composition::IContainerVisual **>(webview_visual.put())));

            auto webview_visual2 = webview_visual.try_as<ABI::Windows::UI::Composition::IVisual2>();
            if (webview_visual2)
                webview_visual2->put_RelativeSizeAdjustment({1.0f, 1.0f});

            winrt::com_ptr<ABI::Windows::UI::Composition::IVisualCollection> children;
            winrt::check_hresult(root->get_Children(children.put()));
            winrt::check_hresult(children->InsertAtTop(webview_visual.get()));
            winrt::check_hresult(_comp->put_RootVisualTarget(webview_visual.get()));
            auto rt_visual = surface.try_as<winrt::Visual>();
            assert(rt_visual);

            m_item = winrt::GraphicsCaptureItem::CreateFromVisual(rt_visual);
            IDXGIDevice *dxgi_device = nullptr;
            winrt::check_hresult(edge->edge_d3d11.device->QueryInterface(&dxgi_device));

            winrt::com_ptr<IInspectable> d3d_device;
            winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
            m_device = d3d_device.as<winrt::IDirect3DDevice>();
            safe_release(dxgi_device);

            m_pixelFormat = winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized;
            m_framePool = winrt::Direct3D11CaptureFramePool::Create(m_device, m_pixelFormat, 2, m_item.Size());
            m_session = m_framePool.CreateCaptureSession(m_item);
            if (m_session)
                m_session.StartCapture();

            update_cursor_mouse();
            winrt::check_hresult(comp->add_CursorChanged(wrl::Callback<ICoreWebView2CursorChangedEventHandler>([&](ICoreWebView2CompositionController *sender, IUnknown *args) -> HRESULT
                                                                                                               {
                                                                                                                   update_cursor_mouse();
                                                                                                                   return S_OK;
                                                                                                               })
                                                             .Get(),
                                                         &CoreWebView2CursorChangedEventRegistrationToken));
        }

        // trigger ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
        ICoreWebView2Controller *_controller = 0;
        _comp->QueryInterface(&_controller);
        auto hr = Invoke(errorCode, _controller);
        safe_release(_controller);

        return hr;
    }

    // ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Controller *_controller) override
    {
        if (FAILED(errorCode))
        {
            handle_failed_edge_load();
            return errorCode;
        }

        winrt::check_hresult(_controller->QueryInterface(&controller));
        ICoreWebView2 *_webview2 = 0;
        winrt::check_hresult(controller->get_CoreWebView2(&_webview2));
        winrt::check_hresult(_webview2->QueryInterface(&webview));
        safe_release(_webview2);

        ICoreWebView2Settings *_settings = 0;
        winrt::check_hresult(webview->get_Settings(&_settings));
        winrt::check_hresult(_settings->QueryInterface(__uuidof(ICoreWebView2Settings7), (void **) &settings));
        safe_release(_settings);

        // configure browser
        winrt::check_hresult(controller->put_ShouldDetectMonitorScaleChanges(false));
        winrt::check_hresult(controller->put_DefaultBackgroundColor(COREWEBVIEW2_COLOR {0}));
        winrt::check_hresult(controller->put_BoundsMode(COREWEBVIEW2_BOUNDS_MODE_USE_RAW_PIXELS));
        winrt::check_hresult(controller->put_RasterizationScale(1.0));
        winrt::check_hresult(controller->put_IsVisible(true));

        winrt::check_hresult(settings->put_AreHostObjectsAllowed(TRUE));
        winrt::check_hresult(settings->put_IsScriptEnabled(TRUE));
        winrt::check_hresult(settings->put_IsWebMessageEnabled(TRUE));
        winrt::check_hresult(settings->put_IsStatusBarEnabled(FALSE));
        winrt::check_hresult(settings->put_IsBuiltInErrorPageEnabled(FALSE));
        winrt::check_hresult(settings->put_AreDefaultContextMenusEnabled(FALSE));
        // winrt::check_hresult(settings->put_AreBrowserAcceleratorKeysEnabled(FALSE));
        winrt::check_hresult(settings->put_AreDefaultScriptDialogsEnabled(FALSE));

        winrt::check_hresult(webview->add_WebMessageReceived(this, &CoreWebView2WebMessageReceivedEventRegistrationToken));
        winrt::check_hresult(webview->add_ProcessFailed(this, &CoreWebView2ProcessFailedEventRegistrationToken));
        winrt::check_hresult(webview->add_ScriptDialogOpening(this, &CoreWebView2ScriptDialogOpeningEventRegistrationToken));
        winrt::check_hresult(webview->add_DocumentTitleChanged(this, &CoreWebView2DocumentTitleChangedEventRegistrationToken));
        winrt::check_hresult(webview->AddScriptToExecuteOnDocumentCreated(L"(function() { console.log('loaded document', window.location.href) })();", NULL));
        winrt::check_hresult(webview->Navigate(L"https://bing.com"));
        // webview->NavigateToString(html);

        // execute some code example
        winrt::check_hresult(webview->ExecuteScript(L"window.chrome.webview.postMessage('example'); return 666;", wrl::Callback<ICoreWebView2ExecuteScriptCompletedHandler>([&](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT
                                                                                                                                                                            {
                                                                                                                                                                                // do something with the result
                                                                                                                                                                                return errorCode;
                                                                                                                                                                            })
                                                                                                                      .Get()));

        // automatically handle permission requests
        winrt::check_hresult(webview->add_PermissionRequested(wrl::Callback<ICoreWebView2PermissionRequestedEventHandler>([&](ICoreWebView2 *sender, ICoreWebView2PermissionRequestedEventArgs *args) -> HRESULT
                                                                                                                          {
                                                                                                                              COREWEBVIEW2_PERMISSION_KIND kind;
                                                                                                                              if (SUCCEEDED(args->get_PermissionKind(&kind)))
                                                                                                                              {
                                                                                                                                  switch (kind)
                                                                                                                                  {
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_FILE_READ_WRITE:
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_AUTOPLAY:
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_LOCAL_FONTS:
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_OTHER_SENSORS:
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
                                                                                                                                  case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
                                                                                                                                  {
                                                                                                                                      args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
                                                                                                                                  }
                                                                                                                                  break;
                                                                                                                                  default:
                                                                                                                                      args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
                                                                                                                                  }
                                                                                                                              }
                                                                                                                              return S_OK;
                                                                                                                          })
                                                                  .Get(),
                                                              &CoreWebView2PermissionRequestedEventRegistrationToken));

        // set window icon as the current favicon
        winrt::check_hresult(webview->add_FaviconChanged(wrl::Callback<ICoreWebView2FaviconChangedEventHandler>([&](ICoreWebView2 *sender, IUnknown *args) -> HRESULT
                                                                                                                {
                                                                                                                    webview->GetFavicon(
                                                                                                                        COREWEBVIEW2_FAVICON_IMAGE_FORMAT_PNG,
                                                                                                                        wrl::Callback<ICoreWebView2GetFaviconCompletedHandler>(
                                                                                                                            [&](HRESULT errorCode, IStream *iconStream) -> HRESULT
                                                                                                                            {
                                                                                                                                if (FAILED(errorCode))
                                                                                                                                    return S_OK;
                                                                                                                                HICON icon = 0;
                                                                                                                                Gdiplus::Bitmap iconBitmap(iconStream);
                                                                                                                                if (iconBitmap.GetHICON(&icon) == Gdiplus::Status::Ok)
                                                                                                                                {
                                                                                                                                    SendMessage(edge->hwnd, WM_SETICON, ICON_SMALL, (LPARAM) icon);
                                                                                                                                }
                                                                                                                                else
                                                                                                                                {
                                                                                                                                    SendMessage(edge->hwnd, WM_SETICON, ICON_SMALL, (LPARAM) IDC_ICON);
                                                                                                                                }
                                                                                                                                return S_OK;
                                                                                                                            })
                                                                                                                            .Get());
                                                                                                                    return S_OK;
                                                                                                                })
                                                             .Get(),
                                                         &CoreWebView2FaviconChangedEventRegistrationToken));

        RECT rc;
        GetClientRect(edge->hwnd, &rc);
        browser->resize(rc.right, rc.bottom);

        return errorCode;
    }

    // ICoreWebView2WebMessageReceivedEventHandler
    virtual HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) override
    {
        // executed when webmessage is posted
        LPWSTR s;
        if (SUCCEEDED(args->TryGetWebMessageAsString(&s)))
            wprintf(L"Message: %s\n", s);
        return S_OK;
    }

    // ICoreWebView2ProcessFailedEventHandler
    virtual HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2 *sender, ICoreWebView2ProcessFailedEventArgs *args) override
    {
        // executed when a browser child process exits unexpectedly
        COREWEBVIEW2_PROCESS_FAILED_KIND fail;
        if (SUCCEEDED(args->get_ProcessFailedKind(&fail)) && fail == COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_EXITED)
        {
            // can we recover?
            release_resources();
            if (FAILED(CreateCoreWebView2Environment(this)))
                exit(-2);
        }
        return S_OK;
    }

    // ICoreWebView2ExecuteScriptCompletedHandler
    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, LPCWSTR resultObjectAsJson) override
    {
        wprintf(L"Message: %s\n", resultObjectAsJson);
        return S_OK;
    }

    // ICoreWebView2ScriptDialogOpeningEventHandler
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2 *sender, ICoreWebView2ScriptDialogOpeningEventArgs *args) override
    {
        // handles js dialogs
        COREWEBVIEW2_SCRIPT_DIALOG_KIND type;
        if (SUCCEEDED(args->get_Kind(&type)))
        {
            switch (type)
            {
            case COREWEBVIEW2_SCRIPT_DIALOG_KIND_ALERT:
            {
                LPWSTR msg;
                LPWSTR str;
                int result;
                if (SUCCEEDED(args->get_Message(&msg)) && webview->get_DocumentTitle(&str))
                    return TaskDialog(edge->hwnd, hinst, str, 0, msg, TDCBF_OK_BUTTON, 0, &result);
            }
            break;
            case COREWEBVIEW2_SCRIPT_DIALOG_KIND_CONFIRM:
            {
                LPWSTR msg;
                int result;
                LPWSTR str;
                if (SUCCEEDED(webview->get_DocumentTitle(&str)) && SUCCEEDED(args->get_Message(&msg)))
                    if (SUCCEEDED(TaskDialog(edge->hwnd, hinst, str, 0, msg, TDCBF_YES_BUTTON | TDCBF_NO_BUTTON, 0, &result)) && IDYES == result)
                        return args->Accept();
            }
            break;
            }
        }
        return S_OK;
    }

    // ICoreWebView2DocumentTitleChangedEventHandler
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2 *sender, IUnknown *args) override
    {
        // set window title as current document title
        LPWSTR str;
        if (SUCCEEDED(webview->get_DocumentTitle(&str)))
            SetWindowTextW(edge->hwnd, str);
        return S_OK;
    }

    // helper: enable reciving mouse events while mouse its outside (offscreen)
    inline int set_capture_mouse(bool enable)
    {
        if (enable)
        {
            if (!capture_on)
                SetCapture(edge->hwnd);
            capture_on = true;
        }
        else
        {
            if (capture_on)
                ReleaseCapture();
            capture_on = false;
        }
        return 0;
    }

    // helper: enable reciving events for mouse leave (offscreen)
    inline void track_mouse()
    {
        if (tracking_on)
            return;
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = edge->hwnd;
        tracking_on = TrackMouseEvent(&tme);
    }

    // helper: send WM_SETCURSOR to window
    inline void update_cursor_mouse()
    {
        PostMessage(edge->hwnd, WM_SETCURSOR, 0, 0);
    }

    // downgrade all the interfaces as possible to add version support
    edge_config *edge {nullptr};
    // ICoreWebView2Environment13*env{ nullptr };
    ICoreWebView2Environment6 *env {nullptr};
    // ICoreWebView2Controller4* controller{ nullptr };
    ICoreWebView2Controller3 *controller {nullptr};
    // ICoreWebView2_22*         webview{ nullptr };
    ICoreWebView2_16 *webview {nullptr};
    // ICoreWebView2Settings7*   settings{ nullptr };
    ICoreWebView2Settings *settings {nullptr};
    // ICoreWebView2CompositionController4* comp{ nullptr };
    ICoreWebView2CompositionController *comp {nullptr};
    // ICoreWebView2SharedBuffer*  shared_buffer{ nullptr };
    ICoreWebView2PrintSettings *print_settings {nullptr};

    EventRegistrationToken CoreWebView2WebMessageReceivedEventRegistrationToken {0};
    EventRegistrationToken CoreWebView2ProcessFailedEventRegistrationToken {0};
    EventRegistrationToken CoreWebView2ScriptDialogOpeningEventRegistrationToken {0};
    EventRegistrationToken CoreWebView2DocumentTitleChangedEventRegistrationToken {0};
    EventRegistrationToken CoreWebView2CursorChangedEventRegistrationToken {0};
    EventRegistrationToken CoreWebView2FaviconChangedEventRegistrationToken {0};
    EventRegistrationToken CoreWebView2PermissionRequestedEventRegistrationToken {0};

    int last_button = 0;
    bool capture_on = false;
    bool tracking_on = false;
    uint64_t reference_count = 1;
    POINT point {0, 0};

    winrt::GraphicsCaptureItem m_item {nullptr};
    winrt::IDirect3DDevice m_device {nullptr};
    winrt::DirectXPixelFormat m_pixelFormat;
    winrt::Direct3D11CaptureFramePool m_framePool {nullptr};
    winrt::GraphicsCaptureSession m_session {nullptr};
    winrt::DispatcherQueueController queue_controller {nullptr};
};

// impl

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WCHAR sz_title[MAX_LOADSTRING];
    WCHAR sz_class[MAX_LOADSTRING];
    static edge_config edge {};
    edge.offscreen = OFFSCREEN_RENDERING;
    hinst = hInstance;
    Gdiplus::GdiplusStartup(&gdip_token, &gdip_input_start, NULL);
#ifdef CONSOLE
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
#endif
    DwmEnableMMCSS(true);
    InitCommonControls();
    LoadStringW(hInstance, IDS_APP_TITLE, sz_title, MAX_LOADSTRING);
    // LoadStringW(hInstance, IDC_BROWSER, sz_class, MAX_LOADSTRING);
    wcscpy_s(sz_class, L"MyEdgeWindowClass");

    // init scope
    {
        WNDCLASSEXW wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT CALLBACK
        {
            // pass messages to browser
            if (browser && edge.offscreen && browser->translate_msg_proc(hWnd, message, wParam, lParam))
                return 0;
            switch (message)
            {
            case WM_MOVE:
                if (browser)
                    browser->update_browser_window();
                break;
            case WM_SIZE:
                if (browser)
                    browser->resize(LOWORD(lParam), HIWORD(lParam));
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        };
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BROWSER));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = sz_class;
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        RegisterClassExW(&wcex);

        edge.hwnd = CreateWindowW(sz_class, sz_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1280, 720, nullptr, nullptr, hInstance, nullptr);
        if (!edge.hwnd)
        {
            DWORD err = GetLastError();
            wchar_t buf[256];
            swprintf_s(buf, L"CreateWindowW 失败，错误码: %lu", err);
            MessageBoxW(NULL, buf, L"错误", MB_OK | MB_ICONERROR);
            return FALSE;
        }

        // set dark theme
        BOOL dmOn = true;
        DwmSetWindowAttribute(edge.hwnd, 20, &dmOn, sizeof(dmOn));
        SetWindowTheme(edge.hwnd, L"DarkMode_Explorer", nullptr);

        ShowWindow(edge.hwnd, SW_SHOW);
        UpdateWindow(edge.hwnd);

        if (OFFSCREEN_RENDERING)
        {
            // create d3d11 device
            constexpr UINT creation_flags {
                D3D11_CREATE_DEVICE_VIDEO_SUPPORT |
                D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS
#ifdef _DEBUG
                | D3D11_CREATE_DEVICE_DEBUG
#endif
            };
            constexpr D3D_FEATURE_LEVEL feature_levels[] = {
                D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2,
                D3D_FEATURE_LEVEL_9_1};
            D3D_FEATURE_LEVEL *ftret = 0;
            winrt::check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creation_flags, feature_levels, 7, D3D11_SDK_VERSION, &edge.edge_d3d11.device, ftret, &context));
            ID3D10Multithread *pMultithread = nullptr;
            winrt::check_hresult(edge.edge_d3d11.device->QueryInterface(IID_PPV_ARGS(&pMultithread)));
            winrt::check_hresult(pMultithread->SetMultithreadProtected(TRUE));
            safe_release(pMultithread);

            IDXGIDevice *dxgi_device = nullptr;
            winrt::check_hresult(edge.edge_d3d11.device->QueryInterface(__uuidof(IDXGIDevice), (void **) &dxgi_device));
            IDXGIAdapter *dxgi_adapter = nullptr;
            winrt::check_hresult(dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void **) &dxgi_adapter));
            IDXGIFactory *dxgi_factory = nullptr;
            winrt::check_hresult(dxgi_adapter->GetParent(__uuidof(IDXGIFactory), (void **) &dxgi_factory));

            // create d3d11 swapchain on window
            DXGI_SWAP_CHAIN_DESC sd {};
            sd.BufferCount = 2;
            sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            sd.BufferDesc.Width = 1280;
            sd.BufferDesc.Height = 720;
            sd.BufferDesc.RefreshRate.Numerator = 0;
            sd.BufferDesc.RefreshRate.Denominator = 0;
            sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            sd.OutputWindow = edge.hwnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            sd.Windowed = true;
            winrt::check_hresult(dxgi_factory->CreateSwapChain(edge.edge_d3d11.device, &sd, &swapchain));
            winrt::check_hresult(dxgi_factory->MakeWindowAssociation(edge.hwnd, 0));

            safe_release(dxgi_factory);
            safe_release(dxgi_adapter);
            safe_release(dxgi_device);
        }

        // create a browser
        browser = new edge_browser(&edge);
    }

    HACCEL accel_table = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BROWSER));
    MSG msg;

    while (true)
    {
        // copy texture on rendertarget texture for easy rendering... normally more complete rendering pipeline has to be done
        if (OFFSCREEN_RENDERING && !IsIconic(edge.hwnd))
        {
            if (browser)
            {
                ID3D11Texture2D *tex = browser->texture();
                if (tex)
                {
                    ID3D11Texture2D *dst = nullptr;
                    if (SUCCEEDED(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &dst)))
                    {
                        context->CopySubresourceRegion(dst, 0, 0, 0, 0, tex, 0, 0);
                        safe_release(dst);
                    }
                    safe_release(tex);
                }
            }

            // present at vsync
            swapchain->Present(1, 0);
        }

        // message loop
        while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, accel_table, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // add some idle time and check if need to exit
        SleepEx(10, false);
        if (msg.message == WM_QUIT)
            break;
    }

    // dispose everithing...
    safe_release(browser);

    if (OFFSCREEN_RENDERING)
    {
        safe_release(edge.edge_d3d11.device);
        safe_release(context);
        safe_release(swapchain);
    }

    Gdiplus::GdiplusShutdown(gdip_token);

#ifdef _DEBUG
    IDXGIDebug *debug = nullptr;
    if (SUCCEEDED(((HRESULT(WINAPI *)(REFIID riid, void *ppDebug))
                       GetProcAddress(LoadLibraryA("DXGIDebug.dll"), "DXGIGetDebugInterface"))(IID_PPV_ARGS(&debug))))
    {
        OutputDebugStringA("*** start DXGI live objects ***\n");
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        OutputDebugStringA("*** end   DXGI live objects ***\n");
        debug->Release();
    }
#endif

    return 0;
}

// use always the dedicated hardware when there are integrated graphics
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")