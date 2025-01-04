////////////////////////////////////////////////////////////////////////////////
// include/data_processor.hpp
//////////////////////////////////////////////////////////////////////////////
#ifndef DATA_PROCESSOR_HPP
#define DATA_PROCESSOR_HPP

#include <string>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

// Forward declarations
class InfluxDBClient;
struct AppData;

/*
 * Handles parsing of MBO data and updates statistics for each data stream.
 */
class DataProcessor {
public:
    DataProcessor(std::shared_ptr<InfluxDBClient> dbClient, AppData* app);
    ~DataProcessor();

    // Processes a response string for a specific streamID
    void processResponse(const std::string &response, const std::string &streamID);

    // Retrieves the list of debug logs
    std::vector<std::string> getDebugLogs();

    // Public error count
    std::atomic<int> errorCount{0};

private:
    std::shared_ptr<InfluxDBClient> db;  // InfluxDB client for data storage
    AppData* appData;                     // Pointer to shared application data
    std::mutex debugMutex;                // Mutex to protect debug logs
    std::vector<std::string> debugLogs;   // Container for debug log entries

    // Tracks statistics and errors for streams
    void incrementStreamError(const std::string &streamID);

    // Adds a debug log entry
    void logDebug(const std::string &reason);

    // Buffer for handling partial JSON inputs (if necessary)
    std::string buffer;
    std::mutex bufferMutex;  // Mutex for thread-safe buffer operations
};

#endif // DATA_PROCESSOR_HPP
