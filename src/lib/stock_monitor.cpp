////////////////////////////////////////////////////////////////////////////////
// src/lib/stock_monitor.cpp
//////////////////////////////////////////////////////////////////////////////
#include "../include/stock_monitor.hpp"
#include "../include/data_processor.hpp"
#include <chrono>
#include <thread>
#include <random>
#include <nlohmann/json.hpp>

// For convenience
using json = nlohmann::json;

StockMonitor::StockMonitor(const Config &cfg, std::shared_ptr<DataProcessor> processor)
    : config(cfg), dataProcessor(processor)
{
}

StockMonitor::~StockMonitor()
{
}

std::string StockMonitor::buildURL()
{
    // Placeholder for building real data source URL
    // This should be implemented based on the actual data source API
    return "http://realdata.source/api";
}

void StockMonitor::run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount)
{
    // Simulate connecting to a real data source
    // For demonstration, we'll generate random data similar to DevMonitor
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> priceDist(100.0, 500.0);
    std::uniform_int_distribution<> qtyDist(1, 1000);

    // Example real ticker symbols
    std::vector<std::string> realTickers = config.symbols;

    while(!stopFlag.load()) {
        for(const auto& ticker : realTickers) {
            double price = priceDist(gen);
            int quantity = qtyDist(gen);
            long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                      std::chrono::system_clock::now().time_since_epoch()).count();

            // Create a mock JSON response
            json response = {
                {"type", "oba"}, // Example type: "oba" for buy
                {"s", ticker},
                {"tm", timestamp},
                {"q", quantity},
                {"p", price},
                {"x", "buy"},
                {"id", "ID" + std::to_string(requestCount.load())},
                {"a", "BrokerA"},
                {"mid", "MID" + std::to_string(requestCount.load())}
            };

            // Process the response
            dataProcessor->processResponse(response.dump(), "REAL");

            requestCount++;
            if(stopFlag.load()) break;
        }

        // Sleep to simulate data rate
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
