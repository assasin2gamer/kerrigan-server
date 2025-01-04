////////////////////////////////////////////////////////////
// dev_data_generator.cpp
//
// Standalone C++ tool to generate dev/test data streams in JSON format.
// Usage example:
//   g++ dev_data_generator.cpp -o dev_data_generator
//   ./dev_data_generator
//
// This simulates data that can be piped or redirected into your system
// for testing with the "DataProcessor" and InfluxDB client.
////////////////////////////////////////////////////////////

#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <string>

// Example of simulating order book data
// We'll produce lines of JSON that mimic real-time feed

int main() {
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_real_distribution<double> priceDist(100.0, 200.0);
    std::uniform_int_distribution<int> qtyDist(1, 1000);
    std::uniform_int_distribution<int> typeDist(0, 4); // simulate different message types

    // Print to stdout repeatedly; consumer can read from here
    // Press Ctrl+C to stop
    while(true) {
        double price = priceDist(rng);
        int quantity = qtyDist(rng);
        long long now = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        // Simulate random message type
        // "oba", "obf", "obc", "obd", "obr"
        // We'll pick a few
        std::string msgType;
        switch(typeDist(rng)) {
            case 0: msgType = "oba"; break;
            case 1: msgType = "obf"; break;
            case 2: msgType = "obc"; break;
            case 3: msgType = "obd"; break;
            default: msgType = "obr"; break;
        }

        // Symbol simulation
        // Here we only simulate "AAPL_S U"
        std::string symbol = "AAPL_S U";

        // Build a JSON string
        // Example: {"type":"oba","s":"AAPL_S U","tm":"123456789","q":"100","p":"123.45"}
        std::cout << "{"
                  << "\"type\":\"" << msgType << "\","
                  << "\"s\":\"" << symbol << "\","
                  << "\"tm\":\"" << now << "\","
                  << "\"q\":\"" << quantity << "\","
                  << "\"p\":\"" << price << "\""
                  << "}" << std::endl;

        // Rate limit
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}
