#include <httplib.h>
#include <nlohmann/json.hpp>

#include "http_server.h"

#include "../steam_login.h"
#include "../task_manager.h"
#include "logger.h"

bool TaskManagerHttpServer::InitHttpServer()
{
    Logger::Log()->info("InitHttpServer");
    httplib::Server svr;

    svr.Get(
        "/f1",
        [](const httplib::Request &req, httplib::Response &res)
        {
            res.set_content("call f1", "text/plain"); // appliation/json
        });

    svr.Post(
        "/steam_login",
        [](const httplib::Request &req, httplib::Response &res)
        {
            auto &sl = SteamLoginSingleton::instance();
            nlohmann::json parsed = nlohmann::json::parse(req.body);
            SteamLogin::LoginInfo login_info = parsed.get<SteamLogin::LoginInfo>();

            std::string err_msg;
            auto login_result = sl.login(login_info, err_msg);
            nlohmann::json ret_json = {
                {"success", login_result},
                {"err_msg", err_msg}};
            res.set_content(ret_json.dump(), "appliation/json");
        });

    svr.Post(
        "/steam_log_out",
        [](const httplib::Request &req, httplib::Response &res)
        {
            auto &sl = SteamLoginSingleton::instance();
            std::string err_msg;
            auto log_out_result = sl.log_out(err_msg);

            nlohmann::json ret_json = {
                {"success", log_out_result},
                {"err_msg", err_msg}};
            res.set_content(ret_json.dump(), "appliation/json");
        });

    svr.Post(
        TaskManagerHttpServer::kDllInitSuccRequestPath,
        [](const httplib::Request &req, httplib::Response &res)
        {
            TaskManagerSingleton::instance().dll_init_succ = true;

            Response response = {true, "success", {}};
            nlohmann::json ret_json = response;
            res.set_content(ret_json.dump(), "appliation/json");
        });

    svr.Post(
        TaskManagerHttpServer::kDllInitFailedRequestPath,
        [](const httplib::Request &req, httplib::Response &res)
        {
            TaskManagerSingleton::instance().dll_init_succ = false;

            Response response = {true, "success", {}};
            nlohmann::json ret_json = response;
            res.set_content(ret_json.dump(), "appliation/json");
        });

    bool res = svr.listen("localhost", TaskManagerHttpServer::kPort);
    Logger::Log()->info("svr.listen return value: {}", res);
    return res;
}