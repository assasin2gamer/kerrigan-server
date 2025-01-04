////////////////////////////////////////////////////////////////////////////////
// config.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/config.hpp"
#include <fstream>
#include <sstream>


Config loadConfig(const std::string &filename) {
    Config cfg;
    cfg.sessionID   = "MY_SESSIONID";
    cfg.influxURL   = "http://localhost:8086";
    cfg.influxDB    = "market_data_dev";
    cfg.totalCores  = 8;
    cfg.reserveCores= 1;
    cfg.dataMode    = DataMode::DEV;

    std::ifstream inFile(filename);
    if(!inFile.is_open()) {
        return cfg;
    }

    std::string line;
    while(std::getline(inFile, line)) {
        if(line.empty()) continue;
        auto pos = line.find('=');
        if(pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        if(key == "session_id") {
            cfg.sessionID = val;
        } else if(key == "influx_url") {
            cfg.influxURL = val;
        } else if(key == "influx_db") {
            cfg.influxDB = val;
        } else if(key == "total_cores") {
            cfg.totalCores = std::stoi(val);
        } else if(key == "reserve_cores") {
            cfg.reserveCores = std::stoi(val);
        } else if(key == "symbols") {
            std::stringstream ss(val);
            std::string s;
            while(std::getline(ss, s, ',')) {
                if(!s.empty()) {
                    cfg.symbols.push_back(s);
                }
            }
        } else if(key == "data_mode") {
            if(val == "DEV") {
                cfg.dataMode = DataMode::DEV;
            } else {
                cfg.dataMode = DataMode::REAL;
            }
        }
    }
    return cfg;
}