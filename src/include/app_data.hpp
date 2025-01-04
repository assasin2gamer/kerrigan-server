///////////////////////////////////////////////////////////////////////////////
// include/app_data.hpp
///////////////////////////////////////////////////////////////////////////////
#ifndef APP_DATA_HPP
#define APP_DATA_HPP

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <gtk/gtk.h> // Included for GtkListStore
#include "config.hpp"

// Structure to hold data for each ticker
struct TickerData {
    std::vector<double> values;
    bool logScale = false; // Flag to determine if log-scale is enabled for this ticker
};

// Structure to hold statistics for each data stream
struct DataStreamStats {
    std::atomic<int> messagesReceived{0};
    std::atomic<int> errors{0};
};

// Main application data structure
struct AppData {
    // Configuration and processing
    Config config; // Uses Config from config.hpp
    std::shared_ptr<class InfluxDBClient> dbClient;
    std::shared_ptr<class DataProcessor> processor;

    // Control flags
    std::atomic<bool> stopFlag{false};
    std::atomic<int> requestCount{0};
    std::atomic<bool> running{false};

    // Graph scaling and panning
    std::atomic<bool> globalLogScale{false}; // Optional global log-scale flag
    double xOffset = 0.0;
    double xScale = 1.0;
    double yScale = 1.0;

    // Time tracking
    double lastTime = 0.0;

    // UI Components
    GtkWidget *labelStats = nullptr;
    GtkWidget *drawingArea = nullptr;
    GtkWidget *textViewDebug = nullptr;

    // Data structures
    std::map<std::string, std::shared_ptr<DataStreamStats>> dataStreamStats;
    std::map<std::string, TickerData> tickerMap;

    // GTK List Store for Data Streams
    GtkListStore* dataStreamsListStore = nullptr; // Added member

    // Mutexes for thread safety
    std::mutex statsMutex;
    std::mutex dataMutex;
    std::mutex debugMutex;

    // Debug logs
    std::vector<std::string> debugLogs;

    // Threads
    std::vector<std::thread> threads;

    // Additional members as needed
};

#endif // APP_DATA_HPP
