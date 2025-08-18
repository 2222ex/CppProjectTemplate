#pragma once

#include "base/stdafx.h"
#include <libipc/ipc.h>

class IPCManager
{
private:
    ipc::channel ipc_channel;

public:
    IPCManager(const std::string &channel_name);
    ~IPCManager();

    virtual void start_recv();
    virtual void process_data(const std::string &data) = 0;
    bool send(const std::string &msg, uint64_t tm = 100u);
};
