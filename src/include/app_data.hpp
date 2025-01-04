////////////////////////////////////////////////////////////////////////////////
// include/app_data.h
////////////////////////////////////////////////////////////////////////////////
#ifndef APP_DATA_H
#define APP_DATA_H

#include <atomic>
#include <mutex>
#include <vector>
#include <thread>
#include <map>
#include <string>
#include <gtk/gtk.h>
#include "config.hpp"
#include "influx_db_client.hpp"
#include "data_processor.hpp"
#include "dev_monitor.hpp"
#include "stock_monitor.hpp"

static const int GRAPH_HISTORY_SIZE = 300;

struct TickerData {
    std::vector<double> values;
};

struct AppData {
    Config config;
    std::shared_ptr<InfluxDBClient> dbClient;
    std::shared_ptr<DataProcessor> processor;
    std::atomic<bool> stopFlag;
    std::atomic<int> requestCount;
    std::vector<std::thread> threads;

    // Graph and UI controls
    GtkWidget* labelStats;
    GtkWidget* drawingArea;
    GtkWidget* textViewDebug;

    // Graph manipulation
    std::map<std::string, TickerData> tickerMap;
    bool logScale;
    double xOffset;
    double xScale;
    double yScale;
    bool running;

    std::mutex dataMutex;
    std::mutex graphMutex;
    double lastTime;
};

#endif // APP_DATA_H
