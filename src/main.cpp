#include "CanDecoder.hpp"
#include "CanReader.hpp"
#include "Config.hpp"
#include "DecodedSignal.hpp"
#include "Logger.hpp"
#include "MqttPublisher.hpp"
#include "SignalJson.hpp"

#include <iostream>
#include <string>

#include <linux/can.h>

int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        std::cerr << "Usage: " << argv[0] << " [config.json]" << std::endl;
        return 2;
    }

    const std::string configPath = argc == 2 ? argv[1] : "config/config.json";

    Config config;

    try
    {
        config = loadConfig(configPath);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Failed to load config: " << error.what() << std::endl;
        return 1;
    }

    CanReader reader(config.canInterface);
    CanDecoder decoder;
    Logger logger(config.logFile);
    MqttPublisher mqtt(config.mqttHost, config.mqttPort, "vehicle-gateway-client");

    if (!mqtt.connect())
    {
        return 1;
    }

    if (!reader.open())
    {
        return 1;
    }

    while (true)
    {
        can_frame frame;
        CanReader::ReadResult result = reader.readFrame(frame);

        if (result == CanReader::ReadResult::Error)
        {
            return 1;
        }

        if (result != CanReader::ReadResult::Frame)
        {
            continue;
        }

        for (const DecodedSignal& signal : decoder.decode(frame))
        {
            if (!signal.valid)
            {
                continue;
            }

            const std::string message = toJson(signal);

            std::cout << message << std::endl;
            logger.log(message);
            mqtt.publish(config.mqttTopic, message);
        }
    }
}
