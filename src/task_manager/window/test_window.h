#ifndef TEST_WINDOW_H
#define TEST_WINDOW_H

#include "singleton.h"
#include "stdafx.h"

#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")

class TestWindow
{
private:

public:
    TestWindow(/* args */);
    ~TestWindow() = default;

    std::string tip_text;
    DWORD target_PID;

    void render_window();
};

class TestWindowSingleton : public Singleton<TestWindow, true>
{
};

#endif