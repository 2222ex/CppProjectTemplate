#include "pattern_util.h"

unsigned long long search_pattern_in_module(MODULEINFO moduleInfo, const std::vector<unsigned short> &pattern)
{
    return search_pattern((unsigned char *) moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage, pattern);
}

unsigned long long search_pattern(const unsigned char *data, const unsigned int data_len, const std::vector<unsigned short> &pattern)
{
    const unsigned char *startAddr = data;
    const unsigned char *endAddr = data + data_len;
    auto it = std::search(
        startAddr, endAddr,
        pattern.begin(), pattern.end(),
        [](unsigned char data, unsigned short pattern)
        {
            return pattern == 0xFFFF || pattern == data;
        });

    if (it != endAddr)
        return (unsigned long long) it;
    else
        return 0;
}