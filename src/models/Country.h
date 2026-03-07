#pragma once
#include <string>

struct Country {
    int         id{0};
    std::string name;
    std::string code;   // ISO-2 code, e.g. "TN", "BE", "GB"
    std::string flag;   // unicode flag emoji
};
