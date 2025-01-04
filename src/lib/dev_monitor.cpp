#include "../include/dev_monitor.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

DevMonitor::DevMonitor(std::shared_ptr<DataProcessor> processor)
    : dataProcessor(processor)
{
}

DevMonitor::~DevMonitor()
{
}

void DevMonitor::run(std::atomic<bool> &stopFlag, std::atomic<int> &requestCount)
{
    std::string buffer; // Buffer to accumulate input

    while (!stopFlag) {
        std::string input;
        if (!std::getline(std::cin, input)) {
            break; // End of input
        }

        // Add new input to the buffer
        buffer += input;

        while (true) {
            try {
                // Look for the start and end of a JSON object
                auto jsonStart = buffer.find('{');
                auto jsonEnd = buffer.find('}', jsonStart);

                if (jsonStart == std::string::npos || jsonEnd == std::string::npos) {
                    break; // No complete JSON object found yet
                }

                // Extract the JSON line
                std::string jsonLine = buffer.substr(jsonStart, jsonEnd - jsonStart + 1);

                // Validate the extracted JSON
                nlohmann::json::parse(jsonLine); // Throws if invalid

                // Process the JSON line
                requestCount++;
                dataProcessor->processResponse(jsonLine, "DEV");

                // Remove processed data from the buffer
                buffer.erase(0, jsonEnd + 1);
            } catch (const nlohmann::json::parse_error &e) {
                // Log and attempt recovery
                std::cerr << "JSON parse error: " << e.what() << " | Buffer content: " << buffer << std::endl;

                // Find the next potential JSON start position
                auto nextStart = buffer.find('{', buffer.find('}') + 1);
                if (nextStart != std::string::npos) {
                    buffer.erase(0, nextStart); // Discard invalid part of the buffer
                } else {
                    buffer.clear(); // No valid JSON start, clear the buffer
                }
                break; // Exit the inner loop to process the next input
            }
        }
    }
}
