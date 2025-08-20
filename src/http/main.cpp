#pragma once

#include "async_http_client.h"
#include "cpp-httplib/http_server.h"
#include "sync_http_client.h"

void async_http_client()
{
    try
    {

        std::string host = "think-async.com";
        std::string path = "/Asio/asio-1.36.0/doc/asio/examples/cpp11_examples.html";

        asio::io_context io_context;
        client c(io_context, host, path);
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }
}

void sync_http_client()
{
    SyncHttpClient cli("localhost", 80);
    std::map<std::string, std::string> http_headers = {
        {"header1", "1"},
        {"header2", "2"},

    };
    std::optional<HttpResponse> opt_res = cli.request(HttpMethod::GET, "/hi", http_headers);
    if (opt_res)
    {
        auto resp = opt_res.value();
        SPDLOG_LOGGER_INFO(Logger::Log(), "Http status: {}", resp.status);
        SPDLOG_LOGGER_INFO(Logger::Log(), "Http content: {}", resp.content);
    }
}

int main(int argc, char *argv[])
{
    std::thread t_http(start_http_server);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    sync_http_client();
    t_http.join();
    return 0;
}