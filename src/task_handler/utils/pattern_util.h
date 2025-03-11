#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "../../base/stdafx.h"

#include <Psapi.h>

unsigned long long search_pattern_in_module(MODULEINFO moduleInfo, const std::vector<unsigned short> &pattern);
unsigned long long search_pattern(const unsigned char *data, const unsigned int data_len, const std::vector<unsigned short> &pattern);
#endif