

#include "../base/logger.h"
#include "../base/stdafx.h"
#include "../task_handler/client_module.h"

#include <MinHook.h>

#include "../task_handler/http/http_server.h"
#include "../task_manager/http/http_server.h"

std::vector<std::shared_ptr<BaseModule>> module_list;

#include <httplib.h>
#include <nlohmann/json.hpp>

struct InitResult
{
    bool is_success;
    std::string err_msg;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InitResult, is_success, err_msg);
};

void Init()
{
    // MessageBoxA(NULL, "Inject!", "Success", MB_OK);
    Logger::Log()->info("Init");

    httplib::Client cli("localhost", TaskManagerHttpServer::kPort);
    std::string err_msg;

    if (MH_Initialize() != MH_OK)
    {
        err_msg = "MH_Initialize failed";
        InitResult init_result = {false, err_msg};
        nlohmann::json json_init_result = init_result;
        cli.Post(TaskManagerHttpServer::kDllInitFailedRequestPath, json_init_result.dump(), "application/json");
        Logger::Log()->error("{}", err_msg);
        return;
    }

    auto &client = ClientModuleSingleton::instance();
    client.InitModuleInfo("client.dll");

    if (client.InitClient(err_msg) == false)
    {
        InitResult init_result = {false, err_msg};
        nlohmann::json json_init_result = init_result;
        cli.Post(TaskManagerHttpServer::kDllInitFailedRequestPath, json_init_result.dump(), "application/json");
        Logger::Log()->error("client init failed: {}", err_msg);
        return;
    }

    module_list.push_back(std::shared_ptr<BaseModule>(&client, [](BaseModule *) {}));

    for (size_t i = 0; i < module_list.size(); i++)
    {
        for (auto &pair : module_list.at(i)->hookInfoMap)
        {
            auto hookInfo = pair.second;
            if (int res = MH_CreateHook(hookInfo.pTarget, hookInfo.pDetour, hookInfo.ppOriginal) != MH_OK)
            {
                err_msg = fmt::format("MH_CreateHook {} failed,status: {}", pair.first, res);
                InitResult init_result = {false, err_msg};
                nlohmann::json json_init_result = init_result;
                cli.Post(TaskManagerHttpServer::kDllInitFailedRequestPath, json_init_result.dump(), "application/json");
                Logger::Log()->error("{}", err_msg);
                return;
            }
            if (MH_EnableHook(hookInfo.pTarget) != MH_OK)
            {
                err_msg = fmt::format("MH_CreateHook {} failed", pair.first);
                InitResult init_result = {false, err_msg};
                nlohmann::json json_init_result = init_result;
                cli.Post(TaskManagerHttpServer::kDllInitFailedRequestPath, json_init_result.dump(), "application/json");
                Logger::Log()->error("{}", err_msg);
                return;
            }
        }
    }

    std::thread http_thread(&TaskHandlerHttpServer::InitHttpServer);
    http_thread.detach();

    err_msg = "";
    InitResult init_result = {true, err_msg};
    nlohmann::json json_init_result = init_result;
    cli.Post(TaskManagerHttpServer::kDllInitSuccRequestPath, json_init_result.dump(), "application/json");
    Logger::Log()->info("dll init success");
}

void Detach()
{
    Logger::Log()->info("Prepare to detach this module");

    for (size_t i = 0; i < module_list.size(); i++)
    {
        for (auto &pair : module_list.at(i)->hookInfoMap)
        {
            auto hookInfo = pair.second;
            if (int res = MH_DisableHook(hookInfo.pTarget) != MH_OK)
            {
                Logger::Log()->error("MH_DisableHook {} failed,status: {}", pair.first, res);
                continue;
            }
        }
    }

    if (MH_Uninitialize() != MH_OK)
    {
        Logger::Log()->error("MH_Uninitialize failed");
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
