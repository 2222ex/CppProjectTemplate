#include "steam_login.h"

#include "launcher.h"

#include <httplib.h>

SteamLogin::SteamLogin(/* args */)
{
    is_before_login_succ = false;
    is_need_before_login = true;
}

SteamLogin::~SteamLogin()
{
}

bool SteamLogin::before_login()
{
    if (is_before_login_succ)
    {
        return true;
    }

    Launcher launcher = LauncherSingleton::instance();

    std::thread thread(
        [&launcher]()
        {
            launcher.launch_application(launcher.AppInfo_steam);
        });
    thread.detach();

    while (launcher.AppInfo_steam.is_launch == false)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    nlohmann::json webSocketDebuggerUrl;

    httplib::Client cli("localhost", 8080);
    if (auto res = cli.Get("/json"))
    {
        if (res->status == 200)
        {
            webSocketDebuggerUrl = nlohmann::json::parse(res->body)[0]["webSocketDebuggerUrl"];
            Logger::Log()->info("webSocketDebuggerUrl: {}", webSocketDebuggerUrl.dump());
        }
        else
        {
            err_msg = fmt::format("Request webSocketDebuggerUrl http status: {}", res->status);
            return false;
        }
    }
    else
    {
        err_msg = fmt::format("Request webSocketDebuggerUrl unknow error");
        return false;
    }

    while (true)
    {
        SteamWebManager stwm(webSocketDebuggerUrl.dump());
        HWND hSteam = FindWindowW(L"SDL_app", L"登录 Steam");
        if (hSteam == NULL)
        {
            err_msg = "Steam window not found";
            continue;
        }
        ShowWindow(hSteam, 5);
        SetForegroundWindow(hSteam);

        std::string str = stwm.getElementInnerHTML(stwm.kAccountUserInputTipPath);
        Logger::Log()->info("str: {}", str);
        if (str == "用帐户名称登录")
        {
            is_before_login_succ = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return true;
}

// 调用该方法前确保steam已经更新完毕及其他操作，处于等待输入账号密码的界面
bool SteamLogin::login(LoginInfo login_info)
{
    if (is_before_login_succ == false)
    {
        err_msg = "Steam is not ready for login";
        return false;
    }

    nlohmann::json webSocketDebuggerUrl;

    httplib::Client cli("localhost", 8080);
    if (auto res = cli.Get("/json"))
    {
        if (res->status == 200)
        {
            webSocketDebuggerUrl = nlohmann::json::parse(res->body)[0]["webSocketDebuggerUrl"];
            Logger::Log()->info("webSocketDebuggerUrl: {}", webSocketDebuggerUrl.dump());
        }
        else
        {
            err_msg = fmt::format("Request webSocketDebuggerUrl http status: {}", res->status);
            return false;
        }
    }
    else
    {
        err_msg = fmt::format("Request webSocketDebuggerUrl unknow error");
        return false;
    }

    try
    {

        SteamWebManager stwm(webSocketDebuggerUrl.dump());

        HWND hSteam = FindWindowW(L"SDL_app", L"登录 Steam");
        if (hSteam == NULL)
        {
            err_msg = "Steam window not found";
            return false;
        }
        ShowWindow(hSteam, 5);
        SetForegroundWindow(hSteam);

        stwm.focusOnElement(stwm.kAccountUserInputPath);

        auto SimulateKeyPress = [](WORD keyCode)
        {
            INPUT inputs[2] = {0};

            // keydown
            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wVk = keyCode;

            // inputs[1].type = INPUT_KEYBOARD;
            // inputs[1].ki.wVk = keyCode;
            // inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

            SendInput(1, inputs, sizeof(INPUT));
        };
        short vk;
        for (size_t i = 0; i < login_info.user.size(); i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // Logger::Log()->info("input char: {}", login_info.user.at(i));
            vk = VkKeyScan(login_info.user.at(i));
            SimulateKeyPress(static_cast<WORD>(vk));
        }

        SimulateKeyPress(VK_TAB);

        for (size_t i = 0; i < login_info.password.size(); i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // Logger::Log()->info("input char: {}", login_info.password.at(i));
            vk = VkKeyScan(login_info.password.at(i));
            SimulateKeyPress(static_cast<WORD>(vk));
        }

        SimulateKeyPress(VK_RETURN);

        stwm.clickElement(stwm.kTokenInputPath);

        for (size_t i = 0; i < login_info.token.size(); i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // Logger::Log()->info("input char: {}", login_info.token.at(i));
            vk = VkKeyScan(login_info.token.at(i));
            SimulateKeyPress(static_cast<WORD>(vk));
        }

        return true;
    }
    catch (const boost::exception &e)
    {
        Logger::Log()->error("boost::exception: {}", boost::diagnostic_information(e));
    }
    catch (const std::exception &e)
    {
        Logger::Log()->error("std::exception: {}", e.what());
    }
    catch (...)
    {
        Logger::Log()->error("unknown exception");
    }

    return false;
}

bool SteamLogin::log_out()
{
    is_before_login_succ = false;
    is_need_before_login = true;
    return true;
}
