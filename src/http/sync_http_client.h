#pragma once

#include "base/stdafx.h"

//
// sync_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "base/stdafx.h"

using asio::ip::tcp;

struct HttpResponse
{
    int status;
    std::string content;
    std::map<std::string, std::string> headers;
};

enum HttpMethod
{
    GET,
    POST,

};

class SyncHttpClient
{
private:
    std::string host;
    int port;
    asio::io_context io_context;

public:
    SyncHttpClient(const std::string &host_, int port_) :
        host(host_), port(port_)
    {
    }

    std::optional<HttpResponse> request(HttpMethod http_method, const std::string &path, const std::map<std::string, std::string> &headers = {})
    {
        try
        {
            // Get a list of endpoints corresponding to the server name.
            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = resolver.resolve(host, "http");

            // Try each endpoint until we successfully establish a connection.
            tcp::socket socket(io_context);
            asio::connect(socket, endpoints);

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "GET " << path << " HTTP/1.0\r\n";
            request_stream << "Host: " << host << "\r\n";
            request_stream << "Accept: */*\r\n";

            for (const auto &pair : headers)
            {
                request_stream << pair.first << ": " << pair.second << "\r\n";
            }

            request_stream << "Connection: close\r\n\r\n";

            // Send the request.
            asio::write(socket, request);

            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.
            asio::streambuf response;
            asio::read_until(socket, response, "\r\n");

            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                std::cout << "Invalid response\n";
                return {};
            }
            HttpResponse resp;
            resp.status = status_code;
            if (status_code != 200)
            {
                std::cout << "Response returned with status code " << status_code << "\n";
                return resp;
            }

            // Read the response headers, which are terminated by a blank line.
            asio::read_until(socket, response, "\r\n\r\n");

            // Process the response headers.
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
            {
                std::cout << header << "\n";
            }

            // Write whatever content we already have to output.
            if (response.size() > 0)
                std::cout << &response;

            // Read until EOF, writing data to output as we go.
            std::error_code error;

            while (asio::read(socket, response, asio::transfer_at_least(1), error))
            {
                resp.content.append(asio::buffers_begin(response.data()), asio::buffers_end(response.data()));
                response.consume(response.size()); // 消费
            }

            if (error != asio::error::eof)
                throw std::system_error(error);

            return resp;
        }
        catch (std::exception &e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }
        return {};
    }

    ~SyncHttpClient()
    {
    }
};
