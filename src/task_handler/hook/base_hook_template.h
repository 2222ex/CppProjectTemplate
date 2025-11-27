#ifndef BASE_HOOK_TEMPLATE_H
#define BASE_HOOK_TEMPLATE_H

#include "base/logger.h"
#include "base/stdafx.h"
#include "hook_id.h"

#include <safetyhook.hpp>

enum class FuncType
{
    fastcall = 0,
    thiscall = 1,
    cdeclcall = 2
};

template<int ID, typename Ret, typename... Args>
class HookInstance
{
public:
    struct CallbackContext
    {
        std::function<Ret(const std::function<Ret(Args...)> &, Args...)> func;
        std::string name;
    };

    inline static HookInstance<ID, Ret, Args...> *instance = nullptr;
    inline static std::mutex hook_mutex; // 保护所有共享状态的全局互斥锁

    // 公共核心逻辑: 构建回调链并执行
    template<typename OriginalCallFunc>
    inline static Ret ExecuteHookChain(OriginalCallFunc originalCall, Args... args)
    {
        // 使用单一锁保护所有访问
        std::lock_guard<std::mutex> lock(hook_mutex);

        if (!instance || !instance->sh_hook)
        {
            return Ret {};
        }

        // 创建调用原始函数的 lambda
        std::function<Ret(Args...)> next = [originalCall](Args... args) -> Ret
        {
            if constexpr (std::is_void_v<Ret>)
            {
                originalCall(args...);
            }
            else
            {
                return originalCall(args...);
            }
        };

        // 构建回调链 (从后往前包装)
        for (auto it = instance->m_callbacks.rbegin(); it != instance->m_callbacks.rend(); ++it)
        {
            auto current = std::move(next);
            next = [current, &ctx = *it](Args... args) -> Ret
            {
                return ctx.func(current, args...);
            };
        }

        // 执行回调链
        if constexpr (std::is_void_v<Ret>)
        {
            next(args...);
        }
        else
        {
            return next(args...);
        }
    }

    // __cdecl 调用约定的 Hook 处理器
    inline static Ret CdeclCallHookHandler(Args... args)
    {
        return ExecuteHookChain(
            [](Args... args) -> Ret
            {
                if constexpr (std::is_void_v<Ret>)
                {
                    instance->sh_hook.call(args...);
                }
                else
                {
                    return instance->sh_hook.call<Ret>(args...);
                }
            },
            args...);
    }

    // __fastcall 调用约定的 Hook 处理器
    inline static Ret SAFETYHOOK_FASTCALL FastcallHookHandler(Args... args)
    {
        return ExecuteHookChain(
            [](Args... args) -> Ret
            {
                if constexpr (std::is_void_v<Ret>)
                {
                    instance->sh_hook.fastcall(args...);
                }
                else
                {
                    return instance->sh_hook.fastcall<Ret>(args...);
                }
            },
            args...);
    }

    std::string m_hookName;
    LPVOID m_target;
    SafetyHookInline sh_hook;
    FuncType func_type;
    std::vector<CallbackContext> m_callbacks;
};

template<int ID, typename Ret, typename... Args>
class HookTemplate : public HookInstance<ID, Ret, Args...>
{
public:
    int InstallHook(std::string hook_name, LPVOID pTarget, FuncType func_type)
    {
        std::lock_guard<std::mutex> lock(HookInstance<ID, Ret, Args...>::hook_mutex);

        this->m_hookName = hook_name;
        this->m_target = pTarget;
        this->func_type = func_type;
        HookInstance<ID, Ret, Args...>::instance = this;

        if (func_type == FuncType::cdeclcall)
        {
            this->sh_hook = safetyhook::create_inline(this->m_target, reinterpret_cast<LPVOID>(&HookInstance<ID, Ret, Args...>::CdeclCallHookHandler));
        }
        else if (func_type == FuncType::fastcall)
        {
            this->sh_hook = safetyhook::create_inline(this->m_target, reinterpret_cast<LPVOID>(&HookInstance<ID, Ret, Args...>::FastcallHookHandler));
        }

        return 0;
    }

    void UninstallHook()
    {
        std::lock_guard<std::mutex> lock(HookInstance<ID, Ret, Args...>::hook_mutex);
        this->sh_hook = {};
        this->m_callbacks.clear();
    }

    void AddHook(std::string name, std::function<Ret(const std::function<Ret(Args...)> &, Args...)> func)
    {
        std::lock_guard<std::mutex> lock(HookInstance<ID, Ret, Args...>::hook_mutex);
        this->m_callbacks.push_back({std::move(func), std::move(name)});
    }

    void RemoveHook(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(HookInstance<ID, Ret, Args...>::hook_mutex);
        auto it = std::remove_if(this->m_callbacks.begin(), this->m_callbacks.end(), [&name](const typename HookInstance<ID, Ret, Args...>::CallbackContext &ctx)
                                 {
                                     return ctx.name == name;
                                 });
        if (it != this->m_callbacks.end())
        {
            this->m_callbacks.erase(it, this->m_callbacks.end());
        }
    }

    void RemoveAllHooks()
    {
        std::lock_guard<std::mutex> lock(HookInstance<ID, Ret, Args...>::hook_mutex);
        this->m_callbacks.clear();
    }

    ~HookTemplate()
    {
        std::lock_guard<std::mutex> lock(HookInstance<ID, Ret, Args...>::hook_mutex);
        HookInstance<ID, Ret, Args...>::instance = nullptr;
    }
};

#endif