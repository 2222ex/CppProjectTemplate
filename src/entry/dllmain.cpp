

#include "../base/logger.h"
#include "../base/stdafx.h"
#include "../task_handler/client_module.h"
#include "../task_handler/panorama_module.h"

#include <MinHook.h>

#include "../task_handler/http_server.h"

std::vector<std::shared_ptr<BaseModule>> module_list;

void Init()
{
    // MessageBoxA(NULL, "Inject!", "Success", MB_OK);
    Logger::Log()->info("Init");

    if (MH_Initialize() != MH_OK)
    {
        Logger::Log()->error("MH_Initialize failed");
        return;
    }

    auto &client = ClientModuleSingleton::instance();
    client.InitModuleInfo("client.dll");
    client.InitClient();
    module_list.push_back(std::shared_ptr<BaseModule>(&client, [](BaseModule *) {}));

    auto &panorama = PanoramaModuleSingleton::instance();
    panorama.InitModuleInfo("panorama.dll");
    panorama.InitPanorama();
    module_list.push_back(std::shared_ptr<BaseModule>(&panorama, [](BaseModule *) {}));

    for (size_t i = 0; i < module_list.size(); i++)
    {
        for (auto &pair : module_list.at(i)->hookInfoMap)
        {
            auto hookInfo = pair.second;
            if (int res = MH_CreateHook(hookInfo.pTarget, hookInfo.pDetour, hookInfo.ppOriginal) != MH_OK)
            {
                Logger::Log()->error("MH_CreateHook {} failed,status: {}", pair.first, res);
                continue;
            }
            if (MH_EnableHook(hookInfo.pTarget) != MH_OK)
            {
                Logger::Log()->error("MH_CreateHook {} failed", pair.first);
                continue;
            }
        }
    }

    std::thread http_thread(&InitHttpServer);
    http_thread.detach();
}

void Detach()
{
    Logger::Log()->info("Prepare to detach this module");
    auto &client = ClientModuleSingleton::instance();

    client.Detach();

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
