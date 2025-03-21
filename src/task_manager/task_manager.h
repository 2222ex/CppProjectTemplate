#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "logger.h"
#include "singleton.h"
#include "stdafx.h"

#include <nlohmann/json.hpp>

#include "../task_handler/auto_open_crate.h"
#include "steam_login.h"

#include <mutex>

enum TaskRunStatus
{
    WAITING,
    RUNNING,
};

class TaskManager
{
private:
    /* data */

public:
    TaskManager(/* args */);
    ~TaskManager() = default;

    struct TaskData
    {
        SteamLogin::LoginInfo login_info;
        AutoOpenCrate::OpenCrateRequest open_crate_request;
    };

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TaskData, login_info, open_crate_request);

    struct RunTaskResult
    {
        mutable std::mutex mtx;

        std::string task_name;
        std::string err_msg;
        bool is_success;

        RunTaskResult() = default;

        RunTaskResult(std::string task_name, std::string err_msg, bool is_success) :
            task_name(std::move(task_name)),
            err_msg(std::move(err_msg)),
            is_success(is_success)
        {
        }

        void update(std::string task_name, std::string err_msg, bool is_success)
        {
            std::unique_lock lock(mtx);
            this->task_name = std::move(task_name);
            this->err_msg = std::move(err_msg);
            this->is_success = is_success;
        }

        RunTaskResult(const RunTaskResult &other)
        {
            std::unique_lock lock_other(other.mtx);
            task_name = other.task_name;
            err_msg = other.err_msg;
            is_success = other.is_success;
        }

        RunTaskResult &operator=(RunTaskResult &&other) noexcept
        {
            if (this != &other)
            {
                std::unique_lock lock_this(mtx, std::defer_lock);
                std::unique_lock lock_other(other.mtx, std::defer_lock);
                std::lock(lock_this, lock_other);

                task_name = std::move(other.task_name);
                err_msg = std::move(other.err_msg);
                is_success = other.is_success;
            }
            return *this;
        }

        // std::string get_task_name() const
        // {
        //     std::unique_lock lock(mtx);
        //     return task_name;
        // }
        // void set_task_name(std::string s)
        // {
        //     std::unique_lock lock(mtx);
        //     task_name = s;
        // }

        // std::string get_err_msg() const
        // {
        //     std::unique_lock lock(mtx);
        //     return err_msg;
        // }
        // void set_err_msg(std::string s)
        // {
        //     std::unique_lock lock(mtx);
        //     err_msg = s;
        // }

        bool get_is_success() const
        {
            std::unique_lock lock(mtx);
            return is_success;
        }
        // void set_is_success(bool s)
        // {
        //     std::unique_lock lock(mtx);
        //     is_success = s;
        // }
    };
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RunTaskResult, task_name, err_msg, is_success);

    void get_task();
    RunTaskResult run_task(std::string data);

    std::atomic<TaskRunStatus> task_run_status;

    std::atomic<bool> dll_init_succ;
};

class TaskManagerSingleton : public Singleton<TaskManager, true>
{
};

#endif