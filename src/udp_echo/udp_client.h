#pragma once

#include "base/stdafx.h"

enum
{
    max_length = 1024
};


using asio::ip::udp;

void echo_udp_client()
{
    try
    {
        std::string host = "127.0.0.1";
        std::string port = "8080";
        asio::io_context io_context;

        udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

        udp::resolver resolver(io_context);
        udp::resolver::results_type endpoints =
            resolver.resolve(udp::v4(), host, port);

        std::cout << "Enter message: ";
        char request[max_length];
        std::cin.getline(request, max_length);
        size_t request_length = std::strlen(request);
        s.send_to(asio::buffer(request, request_length), *endpoints.begin());

        char reply[max_length];
        udp::endpoint sender_endpoint;
        size_t reply_length = s.receive_from(
            asio::buffer(reply, max_length), sender_endpoint);

        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}