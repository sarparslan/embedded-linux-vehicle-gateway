#include "Config.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace
{
    class ConfigTest : public ::testing::Test
    {
    protected:
        std::string writeConfig(const std::string& content)
        {
            path_ = std::filesystem::temp_directory_path() /
                    ("vehicle_gateway_config_" +
                     std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) +
                     ".json");

            std::ofstream file(path_);
            file << content;

            return path_.string();
        }

        void TearDown() override
        {
            if (!path_.empty())
            {
                std::filesystem::remove(path_);
            }
        }

    private:
        std::filesystem::path path_;
    };

    const char* kValidConfig = R"({
        "can_interface": "vcan0",
        "mqtt_host": "localhost",
        "mqtt_port": 1883,
        "mqtt_topic": "vehicles/bus-001/telemetry",
        "log_file": "logs/telemetry.log"
    })";
}

TEST_F(ConfigTest, LoadsValidFile)
{
    Config config = loadConfig(writeConfig(kValidConfig));

    EXPECT_EQ(config.canInterface, "vcan0");
    EXPECT_EQ(config.mqttHost, "localhost");
    EXPECT_EQ(config.mqttPort, 1883);
    EXPECT_EQ(config.mqttTopic, "vehicles/bus-001/telemetry");
    EXPECT_EQ(config.logFile, "logs/telemetry.log");
}

TEST_F(ConfigTest, ThrowsOnMissingFile)
{
    EXPECT_THROW(loadConfig("/nonexistent/config.json"), std::runtime_error);
}

TEST_F(ConfigTest, ThrowsOnInvalidJson)
{
    EXPECT_THROW(loadConfig(writeConfig("{ \"can_interface\": ")), std::runtime_error);
}

TEST_F(ConfigTest, ThrowsOnMissingKey)
{
    EXPECT_THROW(loadConfig(writeConfig(R"({"can_interface": "vcan0"})")), std::runtime_error);
}

TEST_F(ConfigTest, ThrowsOnWrongType)
{
    std::string content = kValidConfig;
    content.replace(content.find("1883"), 4, "\"1883\"");

    EXPECT_THROW(loadConfig(writeConfig(content)), std::runtime_error);
}

TEST_F(ConfigTest, ThrowsOnPortOutOfRange)
{
    std::string content = kValidConfig;
    content.replace(content.find("1883"), 4, "70000");

    EXPECT_THROW(loadConfig(writeConfig(content)), std::runtime_error);
}
