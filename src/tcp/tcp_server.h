#pragma once
#include "base/stdafx.h"

using asio::ip::tcp;

std::string make_daytime_string()
{
    std::time_t now = time(0);
    return std::ctime(&now);
}

void run_tcp_server()
{
    try
    {
        asio::io_context io_context;
        // 监听13端口
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        std::string message = make_daytime_string();
        std::error_code ignored_error;
        asio::write(socket, asio::buffer(message), ignored_error);

        
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}