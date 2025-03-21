#ifndef CLIENT_MODULE_H
#define CLIENT_MODULE_H

#include "../base/logger.h"
#include "../base/singleton.h"
#include "base_module.h"

class ClientModule : public BaseModule
{
private:

public:
    ClientModule(/* args */);
    ~ClientModule() = default;



    struct PatternInfo
    {
        std::string pattern;
        int offset;
        std::function<bool(uint64_t)> func;
    };

    std::map<std::string, PatternInfo> pattern_map;




    bool InitClient(std::string &err_msg);


};

class ClientModuleSingleton : public Singleton<ClientModule, true>
{
};

#endif