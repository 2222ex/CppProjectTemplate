// #include "base/logger.h"
// #include <httplib.h>

// void http_request()
// {
//     httplib::Client cli("www.bing.com");
//     auto req = cli.Get("/search?q=123");
//     if (req)
//     {
//         SPDLOG_LOGGER_INFO(Logger::Log(), "body: {}", req->body);
//     }
// }