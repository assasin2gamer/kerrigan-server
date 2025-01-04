////////////////////////////////////////////////////////////////////////////////
// include/dev_monitor.hpp
//////////////////////////////////////////////////////////////////////////////
#ifndef DEV_MONITOR_HPP
#define DEV_MONITOR_HPP

#include "data_processor.hpp"
#include <atomic>
#include <thread>
#include <memory>
#include <string>

/*
 * Simulates data streaming by generating mock data and feeding it to the DataProcessor.
 */
class DevMonitor {
public:
    DevMonitor(std::shared_ptr<DataProcessor> processor);
    ~DevMonitor();
    
    // Starts the monitoring loop
    void run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount);

private:
    std::shared_ptr<DataProcessor> dataProcessor;
};

#endif // DEV_MONITOR_HPP
