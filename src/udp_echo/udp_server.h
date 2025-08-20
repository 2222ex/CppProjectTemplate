#pragma once

#include "base/stdafx.h"

using asio::ip::udp;

void server(asio::io_context &io_context, unsigned short port)
{
    udp::socket sock(io_context, udp::endpoint(udp::v4(), port));
    while (true)
    {
        char data[max_length];
        udp::endpoint sender_endpoint;
        size_t length = sock.receive_from(
            asio::buffer(data, max_length), sender_endpoint);

        sock.send_to(asio::buffer(data, length), sender_endpoint);
    }
}

void echo_udp_server()
{
    try
    {
        asio::io_context io_context;
        server(io_context, 8080);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return;
}