#include <httplib.h>

#include "test_window.h"

#include "../injector.h"
#include "../launcher.h"
#include "../steam_login.h"

TestWindow::TestWindow()
{
    tip_text = "Waiting";
}

void TestWindow::render_window()
{
    ImGui::Begin("Simple UI", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    auto &injector = InjectorSingleton::instance();

    ImGui::Text("Manually Inject");
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
    ImGui::Separator();

    ImGui::Text("Send Http Request To Game ");
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

    if (ImGui::Button("MyGetItemVectorInfo"))
    {
        if (auto res = cli.Get("/MyGetItemVectorInfo"))
        {
            http_tip = res->body;
        }
    }

    ImGui::Separator();

    ImGui::Text("Launcher");

    if (ImGui::Button("Launch game"))
    {

        std::thread([]()
                    {
                        auto &launcher = LauncherSingleton::instance();
                        launcher.launch_application(launcher.AppInfo_cs2);
                    })
            .detach();
    }

    ImGui::SameLine();
    if (ImGui::Button("Terminate Game"))
    {

        std::thread([]()
                    {
                        auto &launcher = LauncherSingleton::instance();
                        launcher.terminate_application(launcher.AppInfo_cs2);
                    })
            .detach();
    }

    if (ImGui::Button("Launch Steam"))
    {
        std::thread(
            []()
            {
                auto &instance = SteamLoginSingleton::instance();
                SteamLogin::LoginInfo login_info = {
                    "111",
                    "123",
                    "111111"};
                instance.login(login_info);
            })
            .detach();
    }

    if (ImGui::Button("Terminate Steam"))
    {
        auto &launcher = LauncherSingleton::instance();
        launcher.terminate_application(launcher.AppInfo_steam);
    }

    ImGui::End();
}
