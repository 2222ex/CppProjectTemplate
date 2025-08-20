#pragma once

#include "async_http_client.h"
#include "cpp-httplib/http_server.h"

int main(int argc, char *argv[])
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

    return 0;
}