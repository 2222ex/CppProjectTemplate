#include "server_ipc_manager.h"

#include "task.h"

std::shared_ptr<ServerIPCManager> ipc_manager_ptr;

ServerIPCManager::ServerIPCManager(const std::string &channel_name) :
    IPCManager(channel_name)
{
}

ServerIPCManager::~ServerIPCManager()
{
}

void ServerIPCManager::process_data(const std::string &data)
{
    InOutMessage in;
    in.message = data;
    MsgInTaskSingleton::instance().add(std::make_shared<MsgInContent>(std::move(in)));
}