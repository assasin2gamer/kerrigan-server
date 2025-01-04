////////////////////////////////////////////////////////////////////////////////
// data_processor.h
////////////////////////////////////////////////////////////////////////////////
#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <string>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>


class InfluxDBClient;

class DataProcessor {
public:
    DataProcessor(std::shared_ptr<InfluxDBClient> dbClient);
    ~DataProcessor();

    void processResponse(const std::string &response);

    std::atomic<int> errorCount;

    void logDebug(const std::string &reason);
    std::vector<std::string> getDebugLogs();

private:
    std::shared_ptr<InfluxDBClient> db;
    std::mutex debugMutex;
    std::vector<std::string> debugLogs;
};

#endif // DATA_PROCESSOR_H