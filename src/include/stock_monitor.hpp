////////////////////////////////////////////////////////////////////////////////
// include/stock_monitor.hpp
//////////////////////////////////////////////////////////////////////////////
#ifndef STOCK_MONITOR_HPP
#define STOCK_MONITOR_HPP

#include "config.hpp"
#include "data_processor.hpp"
#include <atomic>
#include <string>
#include <memory>

/*
 * Connects to the real data source and processes incoming data.
 */
class StockMonitor {
public:
    StockMonitor(const Config &cfg, std::shared_ptr<DataProcessor> processor);
    ~StockMonitor();
    
    // Starts the monitoring loop
    void run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount);

private:
    Config config;
    std::shared_ptr<DataProcessor> dataProcessor;

    // Helper function to build the data source URL or connection string
    std::string buildURL();
};

#endif // STOCK_MONITOR_HPP
