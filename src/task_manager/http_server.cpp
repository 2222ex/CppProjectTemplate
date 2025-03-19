#include <httplib.h>
#include <nlohmann/json.hpp>

#include "http_server.h"

#include "logger.h"
#include "steam_login.h"

bool InitHttpServer()
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

            auto login_result = sl.login(login_info);
            nlohmann::json ret_json = {
                {"success", login_result},
                {"err_msg", sl.err_msg}};
            res.set_content(ret_json.dump(), "appliation/json");
        });
    bool res = svr.listen("localhost", 24961);
    Logger::Log()->info("svr.listen return value: {}", res);
    return res;
}