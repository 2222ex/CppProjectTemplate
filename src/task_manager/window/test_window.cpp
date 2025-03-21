#include <httplib.h>

#include "test_window.h"

#include "../injector.h"
#include "../launcher.h"

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

    ImGui::End();
}
