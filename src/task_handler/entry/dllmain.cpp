

#include "../task_handler/main_module.h"
#include "../task_handler/rendersystemdx11_module.h"
#include "../task_handler/scenesystem.h"
#include "base/logger.h"
#include "base/stdafx.h"

#include "../task_handler/http/http_server.h"
#include "../task_manager/http/http_server.h"

#include <httplib.h>

struct InitResult
{
    bool is_success;
    std::string err_msg;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InitResult, is_success, err_msg);
};

bool Init()
{
    std::shared_ptr<spdlog::logger> logger = Logger::getLogger("main2", true);
    // MessageBoxA(NULL, "Inject!", "Success", MB_OK);
    SPDLOG_LOGGER_INFO(logger, "Init");

    std::thread http_thread(&TaskHandlerHttpServer::InitHttpServer);
    http_thread.detach();

    httplib::Client cli("localhost", TaskManagerHttpServer::kPort);
    std::string err_msg;

    auto &render = Rendersystemdx11ModuleSingleton::instance();
    render.InitModuleInfo("rendersystemdx11.dll");
    render.Init();

    auto &scene = ScenesystemModuleSingleton::instance();
    scene.InitModuleInfo("scenesystem.dll");
    scene.Init();

    InitResult init_result = {true, err_msg};
    nlohmann::json json_init_result = init_result;
    cli.Post(TaskManagerHttpServer::kDllInitSuccRequestPath, json_init_result.dump(), "application/json");
    SPDLOG_LOGGER_INFO(logger, "dll init success");
    return true;
}

void Detach()
{
    std::shared_ptr<spdlog::logger> logger = Logger::getLogger("main2", true);
    SPDLOG_LOGGER_INFO(logger, "Prepare to detach this module");
}

bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {

        Init();

        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:

        break;
    }
    return TRUE;
}
