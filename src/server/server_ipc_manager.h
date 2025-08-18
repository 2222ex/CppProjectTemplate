#pragma
#include "base/stdafx.h"
#include "ipc/ipc_manager.h"

class ServerIPCManager : public IPCManager
{
private:

public:
    ServerIPCManager(const std::string &channel_name);
    ~ServerIPCManager();
    void process_data(const std::string &data) override;
};

extern std::shared_ptr<ServerIPCManager> ipc_manager_ptr;