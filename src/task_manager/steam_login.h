#ifndef STEAM_LOGIN_H
#define STEAM_LOGIN_H

#include "logger.h"
#include "singleton.h"
#include "stdafx.h"

#include <nlohmann/json.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/exception/all.hpp>

class SteamLogin
{
private:
    /* data */

public:
    SteamLogin(/* args */);
    ~SteamLogin();

    struct LoginInfo
    {
        std::string user;
        std::string password;
        std::string token;
    };

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginInfo, user, password, token)

    bool before_login(std::string &err_msg);
    bool login(LoginInfo login_info, std::string &err_msg);
    bool log_out();

    std::atomic<bool> is_before_login_succ;
    std::atomic<bool> is_need_before_login;
};

class SteamLoginSingleton : public Singleton<SteamLogin, true>
{
};

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

class WebSocketClient
{
public:
    WebSocketClient(asio::io_context &ioc, const std::string &host, const std::string &port, const std::string path) :
        resolver_(ioc), ws_(ioc)
    {
        boost::system::error_code ec;
        auto const results = resolver_.resolve(host, port, ec);
        if (ec)
        {
            Logger::Log()->error("resolve failed: {}", ec.message());
        }

        asio::connect(ws_.next_layer(), results.begin(), results.end(), ec);
        if (ec)
        {
            Logger::Log()->error("asio::connect failed: {}", ec.message());
        }

        ws_.handshake(host, path, ec);
        if (ec)
        {
            Logger::Log()->error("websocket handshake failed: {}", ec.message());
        }
    }

    std::string sendAndReceive(const std::string &message)
    {

        try
        {
            boost::system::error_code ec;
            if (!ws_.is_open())
            {
                Logger::Log()->error("websocket not open!");
                return "";
            }

            ws_.write(asio::buffer(message), ec);
            if (ec)
            {
                Logger::Log()->error("ws_.write failed: {}", ec.message());
                return "";
            }

            beast::flat_buffer buffer;

            ws_.read(buffer, ec);
            if (ec)
            {
                Logger::Log()->error("ws_.read failed: {}", ec.message());
                return "";
            }

            return beast::buffers_to_string(buffer.data());
        }
        catch (const boost::exception &e)
        {
            Logger::Log()->error("boost::exception: {}", boost::diagnostic_information(e));
        }
        catch (const std::exception &e)
        {
            Logger::Log()->error("std::exception: {}", e.what());
        }
    }

    void close()
    {
        boost::system::error_code ec;
        ws_.close(websocket::close_code::normal, ec);
        if (ec)
        {
            Logger::Log()->error("ws_.close failed: {}", ec.message());
        }
    }

    ~WebSocketClient()
    {
        if (ws_.is_open())
        {
            close();
        }
    }
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;

private:
};

class SteamWebManager
{
private:

public:
    const std::string kAccountUserInputPath = "/html/body/div/div[2]/div[2]/div/div[2]/form/div[1]/input";
    const std::string kAccountUserInputTipPath = "/html/body/div/div[2]/div[2]/div/div[2]/form/div[1]/div[1]";
    const std::string kAccountPasswordInputPath = "/html/body/div/div[2]/div[2]/div/div[2]/form/div[2]/input";
    const std::string kTokenInputPath = "/html/body/div/div[2]/div[2]/div/div[2]/div/div[3]/div[1]/div";
    const std::string kLoginButtonPath = "/html/body/div/div[2]/div[2]/div/div[2]/form/div[4]/button";
    const std::string kLoginTipPath = "/html/body/div/div[2]/div[2]/div/div[2]/form/div[5]";
    const std::string kTokenTipPath = "/html/body/div/div[2]/div[2]/div/div[2]/form/div/div[2]/div[1]/div[1]";
    const std::string kNetWorkTipPath = "/html/body/div/div[2]/div[2]/div/div/div/div[1]";

    const std::string kElementNotFoundResult = "[NotFound]";

    std::unique_ptr<WebSocketClient> ws_client;
    asio::io_context ioc;

    SteamWebManager(const std::string webSocketDebuggerUrl)
    {
        Logger::Log()->info("SteamWebManager");

        // 解析 WebSocket URL
        size_t pos = webSocketDebuggerUrl.find("://");
        std::string url = webSocketDebuggerUrl.substr(pos + 3);
        std::string host;
        std::string port;
        std::string path;
        Logger::Log()->info("url: {}", url);
        if (url.find(':') != std::string::npos)
        {
            port = url.substr(url.find(':') + 1, url.find('/') - url.find(':') - 1);
            path = url.substr(url.find('/'));
            path = path.substr(0, path.size() - 1);
            host = url.substr(0, url.find(':'));

            Logger::Log()->info("host: {},port: {},path: {}", host, port, path);
        }

        // 创建 WebSocket 客户端

        ws_client = std::make_unique<WebSocketClient>(ioc, host, port, path);
    }

    ~SteamWebManager()
    {
        if (ws_client && ws_client->ws_.is_open())
        {
            ws_client->close();
            ws_client.reset();
        }
        ioc.stop();
    }

    // action: .innerHTML .focus() .click()
    nlohmann::json build_command(std::string xpath, std::string action)
    {
        // 构造 DevTools 协议命令
        std::string expression = fmt::format(
            "document.evaluate('{}', document, null, XPathResult.FIRST_ORDERED_NODE_TYPE, null).singleNodeValue{}",
            xpath,
            action);

        json command = {
            {"id", 1},
            {"method", "Runtime.evaluate"},
            {"params", {
                           {"expression", expression},
                           //    {"returnByValue", true},

                       }}

        };
        Logger::Log()->trace("build_command: {}", command.dump());
        return command;
    }

    std::string getElement(const std::string xpath, const std::string action)
    {
        std::string response = ws_client->sendAndReceive(build_command(xpath, action).dump());
        return response;
    }

    std::string getElementInnerHTML(const std::string xpath)
    {
        std::string response = ws_client->sendAndReceive(build_command(xpath, ".innerHTML").dump());

        json result = json::parse(response);
        Logger::Log()->info("getElementInnerHTML result: {}", result.dump());
        if (result.contains("result") && result["result"].contains("result") && result["result"]["result"].contains("value"))
        {
            return result["result"]["result"]["value"];
        }

        return kElementNotFoundResult;
    }

    bool focusOnElement(const std::string xpath)
    {
        std::string response = ws_client->sendAndReceive(build_command(xpath, ".focus()").dump());

        return true;
    }

    bool clickElement(const std::string xpath)
    {
        std::string response = ws_client->sendAndReceive(build_command(xpath, ".click()").dump());

        return true;
    }
};

#endif
