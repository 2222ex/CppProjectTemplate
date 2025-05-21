#include <httplib.h>
#include <nlohmann/json.hpp>

#include "http_server.h"

#include "logger.h"

bool TaskManagerHttpServer::InitHttpServer()
{
    SPDLOG_LOGGER_INFO(Logger::Log(), "InitHttpServer");
    httplib::Server svr;

    svr.Get(
        "/f1",
        [](const httplib::Request &req, httplib::Response &res)
        {
            res.set_content("call f1", "text/plain"); // appliation/json
        });

    svr.Post(
        TaskManagerHttpServer::kDllInitSuccRequestPath,
        [](const httplib::Request &req, httplib::Response &res)
        {
            Response response = {true, "success", {}};
            nlohmann::json ret_json = response;
            res.set_content(ret_json.dump(), "appliation/json");
        });

    svr.Post(
        TaskManagerHttpServer::kDllInitFailedRequestPath,
        [](const httplib::Request &req, httplib::Response &res)
        {
            Response response = {true, "success", {}};
            nlohmann::json ret_json = response;
            res.set_content(ret_json.dump(), "appliation/json");
        });

    bool res = svr.listen("localhost", TaskManagerHttpServer::kPort);
    SPDLOG_LOGGER_INFO(Logger::Log(), "svr.listen return value: {}", res);
    return res;
}