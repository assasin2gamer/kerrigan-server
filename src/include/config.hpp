////////////////////////////////////////////////////////////////////////////////
// include/config.hpp
////////////////////////////////////////////////////////////////////////////////
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

enum class DataMode {
    DEV,
    REAL
};

struct Config {
    std::string sessionID;
    std::string influxURL;
    std::string influxDB;
    int totalCores;
    int reserveCores;
    std::vector<std::string> symbols;
    DataMode dataMode;
};

Config loadConfig(const std::string &filename);

#endif // CONFIG_H
