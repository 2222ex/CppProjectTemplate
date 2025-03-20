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

bool SteamLogin::before_login(std::string &err_msg)
{
    if (is_before_login_succ)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    }

    Launcher &launcher = LauncherSingleton::instance();

    if (launcher.AppInfo_steam.is_launch == false)
    {
        std::thread thread(
            [&launcher]()
            {
                launcher.launch_application(launcher.AppInfo_steam);
            });
        thread.detach();

        int count = 1000;
        while (launcher.AppInfo_steam.is_launch == false && count > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count--;
        }
        if (count == 0 && launcher.AppInfo_steam.is_launch == false)
        {
            err_msg = "尝试启动steam失败";
            return false;
        }
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
    if (webSocketDebuggerUrl.empty() || webSocketDebuggerUrl.dump() == "null")
    {
        err_msg = "webSocketDebuggerUrl empty";
        return false;
    }

    SteamWebManager stwm(webSocketDebuggerUrl.dump());
    HWND hSteam = FindWindowW(L"SDL_app", L"登录 Steam");
    if (hSteam == NULL)
    {
        err_msg = "Steam window not found";
        return false;
    }
    // ShowWindow(hSteam, 5);
    // SetForegroundWindow(hSteam);

    std::string str = stwm.getElementInnerHTML(stwm.kAccountUserInputTipPath);
    // std::string str = stwm.getElement(stwm.kAccountUserInputTipPath, "");
    Logger::Log()->info("str: {}", str);
    if (str == "用帐户名称登录")
    {
        Logger::Log()->info("steam ready to login");
        is_before_login_succ = true;
        return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    err_msg = "未找到登录标志";
    return false;
}

// 调用该方法前确保steam已经更新完毕及其他操作，处于等待输入账号密码的界面
bool SteamLogin::login(LoginInfo login_info, std::string &err_msg)
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

        auto SimulateKeyPress = [](WORD keyCode, bool shift = false)
        {
            INPUT inputs[6] = {0};
            int count = 0;

            if (shift)
            {
                inputs[count].type = INPUT_KEYBOARD;
                inputs[count].ki.wVk = VK_SHIFT;
                count++;
            }

            // keydown
            inputs[count].type = INPUT_KEYBOARD;
            inputs[count].ki.wVk = keyCode;
            count++;

            if (shift)
            {
                inputs[count].type = INPUT_KEYBOARD;
                inputs[count].ki.wVk = VK_SHIFT;
                inputs[count].ki.dwFlags = KEYEVENTF_KEYUP;
                count++;
            }

            SendInput(count, inputs, sizeof(INPUT));
        };

        auto SimulateStringInput = [SimulateKeyPress](std::string str)
        {
            for (size_t i = 0; i < str.size(); i++)
            {
                short vk;
                bool isShift = false;
                unsigned char ch = static_cast<unsigned char>(str.at(i));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                if (std::isupper(ch) && std::isdigit(ch) == false)
                {
                    isShift = true;
                }
                else
                {
                    isShift = false;
                }

                vk = VkKeyScan(ch);
                Logger::Log()->info("input char: {},isShift: {},std::isupper: {}", str.at(i), isShift, std::isupper(ch), std::isdigit(ch));
                SimulateKeyPress(static_cast<WORD>(vk), isShift);
            }
        };

        SimulateStringInput(login_info.user);

        SimulateKeyPress(VK_TAB);

        SimulateStringInput(login_info.password);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // SimulateKeyPress(VK_RETURN);
        stwm.clickElement(stwm.kLoginButtonPath);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::string login_tip = stwm.getElementInnerHTML(stwm.kLoginTipPath);
        if (login_tip == "请核对您的密码和帐户名称并重试。")
        {
            err_msg = "账号或密码错误";
            return false;
        }

        stwm.clickElement(stwm.kTokenInputPath);

        SimulateStringInput(login_info.token);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::string tokenTip = stwm.getElementInnerHTML(stwm.kTokenTipPath);
        if (tokenTip == "代码错误，请重试")
        {
            err_msg = "令牌错误";
            return false;
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

bool SteamLogin::log_out(std::string &err_msg)
{
    auto &launcher = LauncherSingleton::instance();
    
    launcher.terminate_application(launcher.AppInfo_steam, err_msg);
    is_before_login_succ = false;
    is_need_before_login = true;
    return true;
}
