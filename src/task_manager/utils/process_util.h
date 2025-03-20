#ifndef PROCESS_UTIL_H
#define PROCESS_UTIL_H

#include "stdafx.h"

#include <Windows.h>

#include <Psapi.h>

DWORD GetProcessPIDByName(std::string process_name, std::string &err_msg);

#endif