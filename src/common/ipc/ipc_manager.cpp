#include "ipc_manager.h"

IPCManager::IPCManager(const std::string &channel_name)
{
    ipc_channel = ipc::channel {channel_name.c_str(), ipc::sender | ipc::receiver};
}

IPCManager::~IPCManager()
{
}

void IPCManager::start_recv()
{
    while (true)
    {

        ipc::buff_t buf = ipc_channel.recv();
        std::string recv_data(buf.get<const char *>(), buf.size() - 1); // 返回的最后一个字符为\0
        if (recv_data.empty() || recv_data[0] == '\0')
        {
            continue;
        }
        process_data(recv_data);
    }
}

bool IPCManager::send(const std::string &msg, uint64_t tm)
{
    return ipc_channel.send(msg, tm);
}
