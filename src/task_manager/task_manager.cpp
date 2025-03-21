#include "task_manager.h"

#include "../task_handler/http/http_server.h"
#include "injector.h"
#include "launcher.h"

#include <httplib.h>

TaskManager::TaskManager()
{
}

// 确保steam已经处于登录界面才get_task
void TaskManager::get_task()
{

    // 获取到任务

    // 启动steam

    // 输入账号密码令牌

    // 启动cs2

    // 分配开箱任务给cs2
}

TaskManager::RunTaskResult TaskManager::run_task(std::string data)
{
    task_run_status = TaskRunStatus::RUNNING;

    std::chrono::steady_clock::time_point task_start_time = std::chrono::steady_clock::now();

    nlohmann::json parse_data = nlohmann::json::parse(data);
    TaskData task_data = parse_data.get<TaskData>();

    auto &sl = SteamLoginSingleton::instance();
    auto &launcher = LauncherSingleton::instance();
    std::string err_msg;

    RunTaskResult run_task_result = {};
    bool isSucc = sl.login(task_data.login_info, err_msg);
    run_task_result.update("steam_login", err_msg, isSucc);

    if (run_task_result.get_is_success() == false)
    {
        return run_task_result;
    }

    auto cs2_launch_start_time = std::chrono::steady_clock::now();

    auto run_game = [&launcher, this, &run_task_result]()
    {
        launcher.AppInfo_cs2.success_call_back = [&launcher]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // waiting cs2 loading to main menu

            auto &injector = InjectorSingleton::instance();
            fs::path dll_path = fs::current_path() / "task_handler.dll";
            std::string err_msg;
            if (injector.InjectUseCreateRemoteThread(dll_path.string().c_str(), launcher.AppInfo_cs2.pi.dwProcessId, err_msg) == 0)
            {
                Logger::Log()->error("inject dll to game failed, err_msg: {}", err_msg);
                launcher.terminate_application(launcher.AppInfo_cs2, err_msg);
            }
            else
            {
                Logger::Log()->info("inject dll to game succ");
            }
        };

        int try_count = 3;
        while (task_run_status == TaskRunStatus::RUNNING && try_count > 0)
        {
            if (launcher.launch_application(launcher.AppInfo_cs2) == false)
            {
                Logger::Log()->info("launch_application {} failed", launcher.AppInfo_cs2.name);
            }

            if (task_run_status == TaskRunStatus::RUNNING)
            {
                Logger::Log()->warn("{} crash while task_run_status is running", launcher.AppInfo_cs2.name);
            }

            try_count--;
        }

        if (task_run_status == TaskRunStatus::RUNNING && try_count == 0)
        {
            run_task_result.update(
                "cs2_restart",
                "task terminate due to cs2 restart too much time",
                false);
        }
    };

    std::thread(run_game).detach();

    while (run_task_result.get_is_success())
    {
        if (dll_init_succ.load() == false)
        {
            continue;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        httplib::Client cli("localhost", TaskHandlerHttpServer::kPort);

        nlohmann::json json_open_crate_request = task_data.open_crate_request;

        if (auto res = cli.Post(TaskHandlerHttpServer::kOpenCrateRequestPath, json_open_crate_request.dump(), "application/json"))
        {
            
        }
    }

    return run_task_result;
}