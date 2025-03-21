#include "http_server.h"

#include "../auto_open_crate.h"
#include "../base/logger.h"
#include "../client_module.h"

#include <MinHook.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

bool g_bIsNeedQuit;
bool g_bIsQuit;

void Detach();

bool TaskHandlerHttpServer::InitHttpServer()
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
                {"count", client.GetItemVectorCount()}};

            res.set_content(resp.dump(), "text/plain"); // appliation/json
        });

    svr.Get(
        "/MyGetItemVectorInfo",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &client = ClientModuleSingleton::instance();
            client.GetItemVectorInfo();

            res.set_content("call MyGetItemVectorInfo", "text/plain"); // appliation/json
        });

    svr.Get(
        "/UseTool",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &client = ClientModuleSingleton::instance();
            char crate[] = "2";
            char key[] = "3";
            client.oUseTool((uintptr_t) client.useToolUnkParam1, key, crate);
            res.set_content("call UseTool", "text/plain"); // appliation/json
        });

    svr.Get(
        "/GetInventory",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &aoc = AutoOpenCrateSingleton::instance();
            aoc.UpdateInventory();
            res.set_content("call UpdateInventory", "text/plain"); // appliation/json
        });

    svr.Get(
        "/OpenCrateTest",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &aoc = AutoOpenCrateSingleton::instance();

            AutoOpenCrate::OpenCrateRequest openCrateRequest;
            openCrateRequest.crate_name = "crate_valve_1";
            openCrateRequest.count = 2;
            openCrateRequest.is_need_tool = true;

            AutoOpenCrate::OpenCrateResult openCrateResult;

            aoc.OpenCrate(openCrateRequest, openCrateResult);

            nlohmann::json json = openCrateResult;
            res.set_content(json.dump(), "text/plain");
        });

    svr.Get(
        "/DumpCrateInfo",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            auto &aoc = AutoOpenCrateSingleton::instance();
            aoc.DumpCrateInfo();
            res.set_content("success", "text/plain");
        });

    svr.Post(
        kOpenCrateRequestPath,
        [&](const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                auto &aoc = AutoOpenCrateSingleton::instance();

                AutoOpenCrate::OpenCrateRequest openCrateRequest = nlohmann::json::parse(req.body).get<AutoOpenCrate::OpenCrateRequest>();

                AutoOpenCrate::OpenCrateResult openCrateResult;

                aoc.OpenCrate(openCrateRequest, openCrateResult);
                nlohmann::json json = openCrateResult;
                Response response = {
                    true,
                    "success",
                    json};

                nlohmann::json json_response = response;

                res.set_content(json_response.dump(), "application/json");
            }
            catch (const std::exception &e)
            {
                Response response = {
                    false,
                    e.what(),
                    {}};

                nlohmann::json json_response = response;

                res.set_content(json_response.dump(), "application/json");
            }
        });

    svr.Get(
        kGetInventoryItemDetail,
        [&](const httplib::Request &req, httplib::Response &res)
        {
            try
            {
                auto &aoc = AutoOpenCrateSingleton::instance();

                nlohmann::json json = aoc.GetInventoryItemDetail();
                Response response = {
                    true,
                    "success",
                    json};

                nlohmann::json json_response = response;

                res.set_content(json_response.dump(), "application/json");
            }
            catch (const std::exception &e)
            {
                Response response = {
                    false,
                    e.what(),
                    {}};

                nlohmann::json json_response = response;

                res.set_content(json_response.dump(), "application/json");
            }
        });

    // svr.Post(
    //     "/f2",
    //     [](const httplib::Request &req, httplib::Response &res)
    //     {
    //         nlohmann::json json = {
    //             {"test", "Test"}};
    //         res.set_content(json.dump(), "appliation/json");
    //     });
    bool res = svr.listen("localhost", kPort);
    Logger::Log()->info("svr.listen return value: {}", res);
    return res;
}