////////////////////////////////////////////////////////////////////////////////
// influx_db_client.cpp
////////////////////////////////////////////////////////////////////////////////
#include "../include/influx_db_client.hpp"
#include <iostream>

InfluxDBClient::InfluxDBClient(const std::string &url, const std::string &dbName)
    : serverURL(url), database(dbName)
{
}

InfluxDBClient::~InfluxDBClient()
{
}

void InfluxDBClient::write(const std::string &measurement,
                           const std::string &symbol,
                           double price,
                           long long timestamp,
                           int quantity,
                           const std::string &side,
                           const std::string &orderID,
                           const std::string &attribution,
                           const std::string &matchID)
{
    std::lock_guard<std::mutex> lock(writeMutex);
    std::cout << "[INFLUX WRITE] meas=" << measurement
              << " symbol=" << symbol
              << " price=" << price
              << " time=" << timestamp
              << " qty=" << quantity
              << " side=" << side
              << " oid=" << orderID
              << " attr=" << attribution
              << " mid=" << matchID
              << " db=" << database
              << " url=" << serverURL
              << std::endl;
}