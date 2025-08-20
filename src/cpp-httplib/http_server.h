#pragma once
#include "base/logger.h"
#include "base/stdafx.h"
bool start_http_server()
{
    httplib::Server svr;

    svr.Get("/hi", [](const httplib::Request &req, httplib::Response &resp)
            {
                for (const auto &header : req.headers)
                {
                    SPDLOG_LOGGER_INFO(Logger::Log(), "header: {}: {}", header.first, header.second);
                }

                resp.set_content("hello", "text/plain");
            });

    return svr.listen("localhost", 80);
}