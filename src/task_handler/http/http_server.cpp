#include "http_server.h"

#include "../base/logger.h"
#include "../client_module.h"

#include <MinHook.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

void Detach();

bool TaskHandlerHttpServer::InitHttpServer()
{
    Logger::Log()->info("InitHttpServer");
    httplib::Server svr;

    svr.Get(
        "/f1",
        [](const httplib::Request &req, httplib::Response &res)
        {
            res.set_content("call f1", "text/plain"); // appliation/json
        });

    svr.Get(
        "/Detach",
        [&](const httplib::Request &req, httplib::Response &res)
        {
            Detach();
            res.set_content("call Detach", "text/plain"); // appliation/json
            svr.stop();
        });

    bool res = svr.listen("localhost", kPort);
    Logger::Log()->info("svr.listen return value: {}", res);
    return res;
}