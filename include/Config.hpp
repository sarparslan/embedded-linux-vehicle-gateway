#pragma once

#include <string>

struct Config
{
    std::string canInterface;
    std::string mqttHost;
    int mqttPort;
    std::string mqttTopic;
    std::string logFile;
};

// Throws std::runtime_error if the file cannot be read, is not valid JSON,
// or is missing a required key.
Config loadConfig(const std::string& filePath);
