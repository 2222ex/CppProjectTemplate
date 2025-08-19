#include "client_ipc_manager.h"

#include "ipc/task.h"

std::shared_ptr<ClientIPCManager> ipc_manager_ptr;

ClientIPCManager::ClientIPCManager(const std::string &channel_name) :
    IPCManager(channel_name)
{
}

ClientIPCManager::~ClientIPCManager()
{
}

void ClientIPCManager::process_data(const std::string &data)
{
    InOutMessage in;
    in.message = data;
    MsgInTaskSingleton::instance().add(std::make_shared<MsgInContent>(std::move(in)));
}