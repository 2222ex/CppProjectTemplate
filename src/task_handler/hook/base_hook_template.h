#ifndef BASE_HOOK_TEMPLATE_H
#define BASE_HOOK_TEMPLATE_H

#include "base/logger.h"
#include "base/stdafx.h"
#include "hook_id.h"

#include <MinHook.h>

template<int ID, typename FuncType, typename Ret, typename... Args>
class HookInstance
{
public:
    struct CallbackContext
    {
        std::function<Ret(const std::function<Ret(Args...)> &, Args...)> func;
        std::string name;
    };

    static HookInstance<ID, FuncType, Ret, Args...> *instance;
    static Ret HookHandler(Args... args);

    std::string m_hookName;
    LPVOID m_target;
    FuncType m_originalFunc;
    std::vector<CallbackContext> m_callbacks;
};

template<int ID, typename FuncType, typename Ret, typename... Args>
HookInstance<ID, FuncType, Ret, Args...> *HookInstance<ID, FuncType, Ret, Args...>::instance = nullptr;

template<int ID, typename FuncType, typename Ret, typename... Args>
Ret HookInstance<ID, FuncType, Ret, Args...>::HookHandler(Args... args)
{
    if (!instance)
    {
        return Ret {};
    }

    std::function<Ret(Args...)> next = [](Args... args) -> Ret
    {
        if constexpr (std::is_void_v<Ret>)
        {
            // SPDLOG_LOGGER_TRACE(Logger::Log(), "Original return void");
            instance->m_originalFunc(args...);
        }
        else
        {

            Ret ret = instance->m_originalFunc(args...);
            // SPDLOG_LOGGER_TRACE(Logger::Log(), "Original has return type: {}", ret);
            // SPDLOG_LOGGER_TRACE(Logger::Log(), "&instance->m_originalFunc: {}", fmt::ptr(&instance->m_originalFunc));
            return ret;
        }
    };

    for (auto it = instance->m_callbacks.rbegin(); it != instance->m_callbacks.rend(); ++it)
    {
        auto current = std::move(next);
        next = [current, &ctx = *it](Args... args) -> Ret
        {
            // SPDLOG_LOGGER_TRACE(Logger::Log(), "Next: {}", ctx.name);
            return ctx.func(current, args...);
        };
    }

    if constexpr (std::is_void_v<Ret>)
    {
        next(args...);
    }
    else
    {
        return next(args...);
    }
}

template<int ID, typename FuncType, typename Ret, typename... Args>
class HookTemplate : public HookInstance<ID, FuncType, Ret, Args...>
{
public:
    MH_STATUS InstallHook(std::string hook_name, LPVOID pTarget)
    {

        this->m_hookName = hook_name;
        this->m_target = pTarget;
        HookInstance<ID, FuncType, Ret, Args...>::instance = this;

        MH_STATUS mh_status = MH_OK;
        if ((mh_status = MH_CreateHook(
                 this->m_target,
                 reinterpret_cast<LPVOID>(&HookInstance<ID, FuncType, Ret, Args...>::HookHandler),
                 reinterpret_cast<LPVOID *>(&this->m_originalFunc))) != MH_OK)
        {
            SPDLOG_LOGGER_WARN(Logger::Log(), "MH_CreateHook {} failed, status: {}", this->m_hookName, (int) mh_status);
            return mh_status;
        }

        if ((mh_status = MH_EnableHook(this->m_target)) != MH_OK)
        {
            SPDLOG_LOGGER_WARN(Logger::Log(), "MH_EnableHook {} failed, status: {}", this->m_hookName, (int) mh_status);
            return mh_status;
        }
        return mh_status;
    }

    void AddHook(std::string name, std::function<Ret(const std::function<Ret(Args...)> &, Args...)> func)
    {
        this->m_callbacks.push_back({std::move(func), std::move(name)});
    }

    ~HookTemplate()
    {
        HookInstance<ID, FuncType, Ret, Args...>::instance = nullptr;
    }
};

#endif