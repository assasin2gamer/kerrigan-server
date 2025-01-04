#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>
#include <mutex>

using json = nlohmann::json;

// Function to get current time in milliseconds since epoch
long long current_timestamp_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

int main(int argc, char* argv[]) {
    std::vector<std::string> symbols = {"AAPL", "GOOG", "MSFT"};
    int messages_per_second = 10; // Adjust as needed
    int duration_seconds = 60;    // Duration to run

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist_type(0, 3);
    std::uniform_int_distribution<int> dist_qty(1, 1000);
    std::uniform_real_distribution<double> dist_price(100.0, 500.0);
    std::uniform_int_distribution<int> dist_side(0, 1);
    std::uniform_int_distribution<int> dist_broker(0, 3);
    std::uniform_int_distribution<int> dist_id(10000, 99999);

    std::vector<std::string> message_types = {"oba", "obf", "obc", "obr"};
    std::vector<std::string> sides = {"buy", "sell"};
    std::vector<std::string> brokers = {"BrokerA", "BrokerB", "BrokerC", "BrokerD"};

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(duration_seconds);
    double interval_ms = 1000.0 / messages_per_second;

    while (std::chrono::steady_clock::now() < end_time) {
        auto msg_start = std::chrono::steady_clock::now();

        // Construct JSON object
        json j;
        try {
            j["type"] = message_types[dist_type(rng)];
            j["s"] = symbols[dist_type(rng) % symbols.size()];
            j["tm"] = current_timestamp_ms();
            j["q"] = dist_qty(rng);
            j["p"] = std::round(dist_price(rng) * 100.0) / 100.0; // Ensure consistent formatting
            j["x"] = sides[dist_side(rng)];
            j["id"] = "ID" + std::to_string(dist_id(rng));
            j["a"] = brokers[dist_broker(rng)];
            j["mid"] = "MID" + std::to_string(dist_id(rng));
            if (j["type"] == "obr") {
                j["nid"] = "NID" + std::to_string(dist_id(rng));
            }

            // Serialize to JSON string
            std::string jsonString = j.dump();

            // Validate JSON by parsing back into an object
            json::parse(jsonString);  // If this throws, the JSON is invalid

            // Thread-safe output
            static std::mutex cout_mutex;
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << jsonString << std::endl;
            }
        } catch (const json::exception& e) {
            // Log and skip invalid JSON
            std::cerr << "JSON generation error: " << e.what() << std::endl;
            continue;
        }

        auto msg_end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = msg_end - msg_start;
        double sleep_time = interval_ms - elapsed.count();
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time)));
        }
    }

    return 0;
}
