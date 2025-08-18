#pragma
#include "base/stdafx.h"
#include "ipc/ipc_manager.h"

class ClientIPCManager : public IPCManager
{
private:

public:
    ClientIPCManager(const std::string &channel_name);
    ~ClientIPCManager();
    void process_data(const std::string &data) override;
};

extern std::shared_ptr<ClientIPCManager> ipc_manager_ptr;