////////////////////////////////////////////////////////////////////////////////
// dev_monitor.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/dev_monitor.hpp"
#include <iostream>
#include <string>

DevMonitor::DevMonitor(std::shared_ptr<DataProcessor> processor)
    : dataProcessor(processor)
{
}

DevMonitor::~DevMonitor()
{
}

void DevMonitor::run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount)
{
    while(!stopFlag) {
        std::string line;
        if(!std::getline(std::cin, line)) {
            break;
        }
        if(!line.empty()) {
            requestCount++;
            dataProcessor->processResponse(line);
        }
    }
}