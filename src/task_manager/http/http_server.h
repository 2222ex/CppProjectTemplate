#ifndef TASK_MANAGER_HTTP_SERVER_H
#define TASK_MANAGER_HTTP_SERVER_H

#include "stdafx.h"
#include "../common/http/http_server.h"

namespace TaskManagerHttpServer
{
const std::string kDllInitSuccRequestPath = "/dll_init_succ";
const std::string kDllInitFailedRequestPath = "/dll_init_failed";
const int kPort = 24961;

bool InitHttpServer();
};

#endif