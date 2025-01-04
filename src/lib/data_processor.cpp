// data_processor.cpp
#include "../include/data_processor.hpp"
#include "../include/influx_db_client.hpp"
#include <json/json.h>
#include <iostream>
#include <cstdlib>
#include "../include/app_data.hpp"
#include <sstream>
#include <mutex>
#include <string>
#include <gtk/gtk.h>

#include <nlohmann/json.hpp>
// For convenience

// For convenience
using json = nlohmann::json;

DataProcessor::DataProcessor(std::shared_ptr<InfluxDBClient> dbClient, AppData* app)
    : db(dbClient), appData(app), errorCount(0)
{
}

DataProcessor::~DataProcessor()
{
}

void DataProcessor::processResponse(const std::string &response, const std::string &streamID) {
    try {
        logDebug("Received response: " + response);

        // Attempt to parse the response
        json root;
        try {
            root = json::parse(response);
        } catch (const json::parse_error &e) {
            errorCount++;
            logDebug("JSON parse error: " + std::string(e.what()) + " | Raw response: " + response);
            incrementStreamError(streamID);
            return;
        }

        // Validate JSON structure
        if (!root.is_object()) {
            errorCount++;
            logDebug("Invalid JSON structure. Not an object. | Raw response: " + response);
            incrementStreamError(streamID);
            return;
        }

        // Required fields
        std::vector<std::string> requiredFields = {
            "type", "s", "tm", "q", "p", "x", "id", "a", "mid"
        };

        for (const auto &field : requiredFields) {
            if (!root.contains(field)) {
                errorCount++;
                logDebug("Missing required field: " + field + " | Raw response: " + response);
                incrementStreamError(streamID);
                return;
            }
        }

        // Extract fields
        std::string msgType = root["type"].get<std::string>();
        std::string symbol = root["s"].get<std::string>();
        long long timestamp = root["tm"].get<long long>();
        int quantity = root["q"].get<int>();
        double price = root["p"].get<double>();
        std::string side = root["x"].get<std::string>();
        std::string orderID = root["id"].get<std::string>();
        std::string attr = root["a"].get<std::string>();
        std::string matchID = root["mid"].get<std::string>();

        // Increment stream stats
        {
            std::lock_guard<std::mutex> lock(appData->statsMutex);
            auto it = appData->dataStreamStats.find(streamID);
            if (it == appData->dataStreamStats.end()) {
                // Insert a new DataStreamStats object if not present
                auto emplaceResult = appData->dataStreamStats.emplace(
                    streamID, std::make_shared<DataStreamStats>());
                if (!emplaceResult.second) {
                    logDebug("Failed to emplace DataStreamStats for stream: " + streamID + " | Raw response: " + response);
                    return;
                }
                it = emplaceResult.first;
            }
            // Increment messagesReceived
            it->second->messagesReceived++;
        }
         // Add ticker data for graphing
        {
            std::lock_guard<std::mutex> lock(appData->dataMutex);
            if(appData->tickerMap.find(symbol) == appData->tickerMap.end()) {
                appData->tickerMap[symbol] = TickerData();
            }
            appData->tickerMap[symbol].values.push_back(price);

            // Optional: Limit the size of the values vector to prevent unbounded growth
            if(appData->tickerMap[symbol].values.size() > 1000) { // Adjust the limit as needed
                appData->tickerMap[symbol].values.erase(appData->tickerMap[symbol].values.begin(),
                                                       appData->tickerMap[symbol].values.end() - 1000);
            }
        }

        // Route events
        if (msgType == "oba" || msgType == "obf" || msgType == "obc" || msgType == "obd" || msgType == "obb") {
            db->write("order_book", symbol, price, timestamp, quantity, side, orderID, attr, matchID);
        } else if (msgType == "obr") {
            std::string newID = root.value("nid", "");
            db->write("order_book", symbol, price, timestamp, quantity, side, newID, attr, matchID);
        } else {
            logDebug("Unhandled message type: " + msgType + " | Raw response: " + response);
        }
    } catch (const std::exception &e) {
        errorCount++;
        logDebug("Exception caught: " + std::string(e.what()) + " | Raw response: " + response);
        incrementStreamError(streamID);
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

void DataProcessor::incrementStreamError(const std::string &streamID)
{
    if (streamID.empty()) return;

    std::lock_guard<std::mutex> lock(appData->statsMutex);
    auto it = appData->dataStreamStats.find(streamID);
    if (it != appData->dataStreamStats.end()) {
        it->second->errors++;
    }
}