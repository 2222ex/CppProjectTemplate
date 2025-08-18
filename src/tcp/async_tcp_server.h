#pragma once

#include "base/stdafx.h"

using asio::ip::tcp;

class AsyncTcpServer
{
private:
asio::io_context io_context_ public :
    AsyncTcpServer(asio::io_context &io_context) :
    io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        start_accept();
    }

    ~AsyncTcpServer()
    {
    }

    void start_accept()
    {
        tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(), std::bind(&tcp_server::handle_accept, this, new_connection, asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection, const std::error_code &error)
    {
        if (!error)
        {
            new_connection->start();
        }

        start_accept();
    }
};
