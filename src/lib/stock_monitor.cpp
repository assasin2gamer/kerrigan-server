////////////////////////////////////////////////////////////////////////////////
// stock_monitor.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/stock_monitor.hpp"
#include <curl/curl.h>
#include <sstream>
#include <iostream>


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    auto *response = static_cast<std::string*>(userp);
    response->append((char*)contents, size * nmemb);
    return size * nmemb;
}

StockMonitor::StockMonitor(const Config &cfg, std::shared_ptr<DataProcessor> processor)
    : config(cfg), dataProcessor(processor)
{
}

StockMonitor::~StockMonitor()
{
}

std::string StockMonitor::buildURL() {
    std::stringstream ss;
    ss << "https://api.activetick.com/stream.json?sessionid=" << config.sessionID << "&symbols=";
    for(size_t i = 0; i < config.symbols.size(); ++i) {
        ss << config.symbols[i];
        if(i < config.symbols.size() - 1) {
            ss << ",";
        }
    }
    return ss.str();
}

void StockMonitor::run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount) {
    CURL *curl = curl_easy_init();
    if(!curl) {
        dataProcessor->logDebug("CURL init failed");
        return;
    }
    std::string url = buildURL();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    while(!stopFlag) {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            requestCount++;
            dataProcessor->processResponse(response);
        } else {
            dataProcessor->errorCount++;
            dataProcessor->logDebug("CURL error: " + std::string(curl_easy_strerror(res)));
        }
    }
    curl_easy_cleanup(curl);
}