#pragma once
#include <string>

struct Radio {
    int         id{0};
    std::string name;
    std::string streamUrl;
    std::string genre;
    std::string description;
    std::string logoUrl;
    int         countryId{0};
};
