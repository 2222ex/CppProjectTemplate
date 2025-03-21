#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../base/logger.h"
#include "../base/singleton.h"
#include "../base/stdafx.h"

#include <mutex>

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

    struct AppInfo
    {
        std::mutex mtx;

        std::string name;
        std::string path;
        std::string launch_param;

        PROCESS_INFORMATION pi;

        DWORD exit_code;
        std::atomic<bool> is_launch;

        std::function<void()> success_call_back;
    };

    AppInfo AppInfo_cs2;

    bool launch_application(AppInfo &appInfo);
    bool terminate_application(AppInfo &appInfo, std::string &err_msg);
};

class LauncherSingleton : public Singleton<Launcher, true>
{
};

#endif