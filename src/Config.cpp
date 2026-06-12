#include "Config.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace
{
    template <typename T>
    T readKey(const nlohmann::json& json, const std::string& key)
    {
        if (!json.contains(key))
        {
            throw std::runtime_error("missing key '" + key + "'");
        }

        try
        {
            return json.at(key).get<T>();
        }
        catch (const nlohmann::json::exception&)
        {
            throw std::runtime_error("key '" + key + "' has the wrong type");
        }
    }
}

Config loadConfig(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        throw std::runtime_error("cannot open config file " + filePath);
    }

    nlohmann::json json;

    try
    {
        file >> json;
    }
    catch (const nlohmann::json::parse_error& error)
    {
        throw std::runtime_error("invalid JSON in " + filePath + ": " + error.what());
    }

    Config config {
        readKey<std::string>(json, "can_interface"),
        readKey<std::string>(json, "mqtt_host"),
        readKey<int>(json, "mqtt_port"),
        readKey<std::string>(json, "mqtt_topic"),
        readKey<std::string>(json, "log_file")
    };

    if (config.mqttPort < 1 || config.mqttPort > 65535)
    {
        throw std::runtime_error("mqtt_port must be between 1 and 65535");
    }

    return config;
}
