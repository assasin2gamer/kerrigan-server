////////////////////////////////////////////////////////////////////////////////
// dev_monitor.h
////////////////////////////////////////////////////////////////////////////////
#ifndef DEV_MONITOR_H
#define DEV_MONITOR_H

#include <atomic>
#include <thread>
#include <memory>
#include <string>
#include "data_processor.hpp"

class DevMonitor {
public:
    DevMonitor(std::shared_ptr<DataProcessor> processor);
    ~DevMonitor();
    void run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount);

private:
    std::shared_ptr<DataProcessor> dataProcessor;
};

#endif // DEV_MONITOR_H