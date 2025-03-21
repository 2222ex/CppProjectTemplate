#ifndef COMMON_HTTP_SERVER_H
#define COMMON_HTTP_SERVER_H

#include <nlohmann/json.hpp>

struct Response
{
    bool success;
    std::string err_msg;
    nlohmann::json data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Response, success, err_msg, data);
};

#endif