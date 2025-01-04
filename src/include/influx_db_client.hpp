////////////////////////////////////////////////////////////////////////////////
// influx_db_client.h
////////////////////////////////////////////////////////////////////////////////
#ifndef INFLUX_DB_CLIENT_H
#define INFLUX_DB_CLIENT_H

#include <string>
#include <mutex>
#include <memory>

class InfluxDBClient {
public:
    InfluxDBClient(const std::string &url, const std::string &dbName);
    ~InfluxDBClient();

    void write(const std::string &measurement,
               const std::string &symbol,
               double price,
               long long timestamp,
               int quantity,
               const std::string &side,
               const std::string &orderID,
               const std::string &attribution,
               const std::string &matchID);

private:
    std::string serverURL;
    std::string database;
    std::mutex writeMutex;
};

#endif // INFLUX_DB_CLIENT_H