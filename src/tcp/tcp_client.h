#pragma once

#include "base/stdafx.h"

using asio::ip::tcp;

void run_tcp_client()
{
    try
    {
        std::string host = "localhost";
        asio::io_context io_context;

        tcp::resolver resolver(io_context);

        // 这行代码调用 `resolver` 的 `resolve` 方法，将主机名 `host` 和服务名 `"daytime"` 解析为一个或多个 TCP 端点（即 IP 地址和端口号的组合）。
        // 返回值类型为 `tcp::resolver::results_type`，它是一个包含所有解析结果的集合。
        // `resolve` 方法会根据提供的主机名和服务名查找对应的网络地址。例如，`"daytime"` 是一个标准服务名，通常对应于 13 号端口。
        // 解析结果存储在 `endpoints` 变量中，后续可以用这些端点尝试建立 TCP 连接。这一步是网络编程中常见的域名解析过程，确保客户端能够找到目标服务器的实际地址。
        tcp::resolver::results_type endpoints = resolver.resolve(host, "daytime");

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);
        while (true)
        {
            std::array<char, 128> buf;
            std::error_code error;

            size_t len = socket.read_some(asio::buffer(buf), error);

            if (error == asio::error::eof)
            {
                break;
            }
            else if (error)
            {
                throw std::system_error(error);
            }

            std::cout.write(buf.data(), len);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}