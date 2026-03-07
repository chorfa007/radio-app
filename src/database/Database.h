#pragma once
#include "../models/Country.h"
#include "../models/Radio.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <functional>

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    // Prevent copy
    Database(const Database&)            = delete;
    Database& operator=(const Database&) = delete;

    void initSchema();
    void seedData();

    std::vector<Country>          getCountries() const;
    std::vector<Radio>            getRadiosByCountry(int countryId) const;
    std::optional<Radio>          getRadioById(int radioId) const;
    bool                          isSeeded() const;

private:
    sqlite3*    m_db{nullptr};

    void exec(const std::string& sql) const;
};
