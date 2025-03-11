#include "data_transform_util.h"

std::vector<unsigned short> hexstring2shorts(const std::string &hex)
{
    std::vector<unsigned short> shorts;

    std::istringstream iss(hex);
    std::string byteString;
    while (iss >> byteString)
    {
        if (byteString == "??")
        {
            shorts.push_back(0xFFFF);
        }
        else
        {
            unsigned short byteValue;
            std::istringstream(byteString) >> std::hex >> byteValue;
            shorts.push_back(byteValue);
        }
    }
    return shorts;
}