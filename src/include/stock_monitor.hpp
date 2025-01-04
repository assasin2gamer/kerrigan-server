////////////////////////////////////////////////////////////////////////////////
// stock_monitor.h
////////////////////////////////////////////////////////////////////////////////
#ifndef STOCK_MONITOR_H
#define STOCK_MONITOR_H

#include "config.hpp"
#include "data_processor.hpp"
#include <atomic>
#include <string>
#include <memory>


class StockMonitor {
public:
    StockMonitor(const Config &cfg, std::shared_ptr<DataProcessor> processor);
    ~StockMonitor();
    void run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount);

private:
    Config config;
    std::shared_ptr<DataProcessor> dataProcessor;
    std::string buildURL();
};

#endif // STOCK_MONITOR_H