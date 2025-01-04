////////////////////////////////////////////////////////////////////////////////
// data_processor.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/data_processor.hpp"
#include "../include/influx_db_client.hpp"
#include <json/json.h>
#include <iostream>
#include <cstdlib>
#include <cstdlib>


DataProcessor::DataProcessor(std::shared_ptr<InfluxDBClient> dbClient)
    : db(dbClient)
{
    errorCount.store(0);
}

DataProcessor::~DataProcessor()
{
}

void DataProcessor::processResponse(const std::string &response)
{
    Json::Reader reader;
    Json::Value root;
    if(!reader.parse(response, root)) {
        errorCount++;
        logDebug("JSON parse error");
        return;
    }

    std::string msgType  = root.get("type", "").asString();
    std::string symbol   = root.get("s", "").asString();
    long long timestamp  = std::atoll(root.get("tm", "0").asCString());
    int quantity         = std::atoi(root.get("q", "0").asCString());
    double price         = std::atof(root.get("p", "0").asCString());
    std::string side     = root.get("x", "").asString();
    std::string orderID  = root.get("id", "").asString();
    std::string attr     = root.get("a", "").asString();
    std::string matchID  = root.get("mid", "").asString();

    if(msgType.empty()) {
        logDebug("Empty message type");
    }

    if(msgType == "oba" || msgType == "obf" || msgType == "obc" || msgType == "obd" || msgType == "obb") {
        db->write("order_book", symbol, price, timestamp, quantity, side, orderID, attr, matchID);
    }
    else if(msgType == "obr") {
        std::string newID = root.get("nid", "").asString();
        db->write("order_book", symbol, price, timestamp, quantity, side, newID, attr, matchID);
    }
    else {
        logDebug("Unhandled message type: " + msgType);
    }
}

void DataProcessor::logDebug(const std::string &reason)
{
    std::lock_guard<std::mutex> lock(debugMutex);
    debugLogs.push_back(reason);
}

std::vector<std::string> DataProcessor::getDebugLogs()
{
    std::lock_guard<std::mutex> lock(debugMutex);
    return debugLogs;
}