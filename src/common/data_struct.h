#ifndef COMMON_DATA_STRUCT_H
#define COMMON_DATA_STRUCT_H

#include <nlohmann/json.hpp>

struct ItemDetail
{
    uint64_t item_id;
    std::string valve_def_name;
    std::string correct_key_name;
    bool is_found_correct_key;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ItemDetail, item_id, valve_def_name, correct_key_name, is_found_correct_key)
};

#endif