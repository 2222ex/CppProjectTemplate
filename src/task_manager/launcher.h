#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../base/logger.h"
#include "../base/singleton.h"
#include "../base/stdafx.h"

class Launcher
{
private:
    std::string err_msg;

    std::shared_ptr<spdlog::logger> log;

public:
    Launcher(/* args */);
    ~Launcher() = default;

    const std::string kGameLauncherPath = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\game\\bin\\win64\\cs2.exe";
    const std::string kGameLaunchParam = "-steam -worldwide -insecure";

    const std::string kSteamLauncherPath = "C:\\Program Files (x86)\\Steam\\steam.exe";
    const std::string kSteamLauncherParam = "";

    struct AppInfo
    {
        std::string name;
        std::string path;
        std::string launch_param;

        PROCESS_INFORMATION game_pi;
        DWORD exit_code;
        bool is_launch;

        std::function<void()> success_call_back;
    }; // AppInfo_steam, AppInfo_cs2

    AppInfo AppInfo_steam;
    AppInfo AppInfo_cs2;

    bool launch_application(AppInfo &appInfo);
    bool terminate_application(AppInfo &appInfo);
};

class LauncherSingleton : public Singleton<Launcher, true>
{
};

#endif