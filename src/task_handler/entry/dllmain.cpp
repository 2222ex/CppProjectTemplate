

#include "../task_handler/main_module.h"
#include "base/logger.h"
#include "base/stdafx.h"

#include <MinHook.h>

#include "../task_handler/http/http_server.h"
#include "../task_manager/http/http_server.h"

#include <httplib.h>

struct InitResult
{
    bool is_success;
    std::string err_msg;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InitResult, is_success, err_msg);
};

void Init()
{
    // MessageBoxA(NULL, "Inject!", "Success", MB_OK);
    SPDLOG_LOGGER_INFO(Logger::Log(), "Init");

    httplib::Client cli("localhost", TaskManagerHttpServer::kPort);
    std::string err_msg;

    if (MH_Initialize() != MH_OK)
    {
        err_msg = "MH_Initialize failed";
        InitResult init_result = {false, err_msg};
        nlohmann::json json_init_result = init_result;
        cli.Post(TaskManagerHttpServer::kDllInitFailedRequestPath, json_init_result.dump(), "application/json");
        SPDLOG_LOGGER_ERROR(Logger::Log(), "{}", err_msg);
        return;
    }

    auto &main = MainModuleSingleton::instance();
    main.InitModuleInfo();

    if (main.InitClient(err_msg) == false)
    {
        InitResult init_result = {false, err_msg};
        nlohmann::json json_init_result = init_result;
        cli.Post(TaskManagerHttpServer::kDllInitFailedRequestPath, json_init_result.dump(), "application/json");
        SPDLOG_LOGGER_ERROR(Logger::Log(), "main init failed: {}", err_msg);
        return;
    }

    std::thread http_thread(&TaskHandlerHttpServer::InitHttpServer);
    http_thread.detach();

    err_msg = "";
    InitResult init_result = {true, err_msg};
    nlohmann::json json_init_result = init_result;
    cli.Post(TaskManagerHttpServer::kDllInitSuccRequestPath, json_init_result.dump(), "application/json");
    SPDLOG_LOGGER_INFO(Logger::Log(), "dll init success");
}

void Detach()
{
    SPDLOG_LOGGER_INFO(Logger::Log(), "Prepare to detach this module");

    if (MH_Uninitialize() != MH_OK)
    {
        SPDLOG_LOGGER_ERROR(Logger::Log(), "MH_Uninitialize failed");
    }
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
