
// #include "base/logger.h"
// #define CPPHTTPLIB_OPENSSL_SUPPORT
// #include <httplib.h>

// void https_request()
// {
//     httplib::SSLClient cli("www.bing.com");
//     cli.enable_server_certificate_verification(false);
//     auto req = cli.Get("/search?q=123");
//     if (req)
//     {
//         SPDLOG_LOGGER_INFO(Logger::Log(), "body: {}", req->body);
//     }
// }