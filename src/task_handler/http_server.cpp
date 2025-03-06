#include "http_server.h"

#include "../base/logger.h"
#include "client_module.h"

#include <MinHook.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

bool g_bIsNeedQuit;
bool g_bIsQuit;

void Detach()
{
    Logger::Log()->info("Prepare to detach this module");
    auto &client = ClientModuleSingleton::instance();
    client.Detach();
    if (MH_Uninitialize() != MH_OK)
    {
        Logger::Log()->error("MH_Uninitialize failed");
    }
}

bool InitHttpServer()
{
    Logger::Log()->info("InitHttpServer");
    httplib::Server svr;

    g_bIsNeedQuit = false;
    g_bIsQuit = false;

    svr.Get(
        "/f1",
        [](const httplib::Request &req, httplib::Response &res)
        {
            res.set_content("call f1", "text/plain"); // appliation/json
        });

    svr.Get(
        "/DumpInventoryToConsole",
        [](const httplib::Request &req, httplib::Response &res)
        {
            auto &client = ClientModuleSingleton::instance();
            client.DumpInventoryToConsole(client.MyGetLocalCSInventory(), true);
            res.set_content("call DumpInventoryToConsole", "text/plain"); // appliation/json
        });

    svr.Get(
        "/Detach",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            Detach();
            res.set_content("call Detach", "text/plain"); // appliation/json
            svr.stop();
        });

    svr.Get(
        "/GetInventoryCount",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &client = ClientModuleSingleton::instance();
            nlohmann::json resp = {
                {"count", client.MyGetItemVectorCount()}};

            res.set_content(resp.dump(), "text/plain"); // appliation/json
        });

    svr.Get(
        "/MyGetItemVectorInfo",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &client = ClientModuleSingleton::instance();
            client.MyGetItemVectorInfo();

            res.set_content("call MyGetItemVectorInfo", "text/plain"); // appliation/json
        });

    svr.Get(
        "/UseTool",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &client = ClientModuleSingleton::instance();
            char crate[] = "42367912968";
            char key[] = "42367944202";
            client.oUseTool(0x162c5491e00, key, crate);
            res.set_content("call UseTool", "text/plain"); // appliation/json
        });

    // svr.Post(
    //     "/f2",
    //     [](const httplib::Request &req, httplib::Response &res)
    //     {
    //         nlohmann::json json = {
    //             {"test", "Test"}};
    //         res.set_content(json.dump(), "appliation/json");
    //     });
    bool res = svr.listen("localhost", 24960);
    Logger::Log()->info("svr.listen return value: {}", res);
    return res;
}