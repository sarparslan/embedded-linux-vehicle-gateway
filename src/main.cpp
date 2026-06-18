#include "CanDecoder.hpp"
#include "CanReader.hpp"
#include "Config.hpp"
#include "DecodedSignal.hpp"
#include "Logger.hpp"
#include "MqttPublisher.hpp"
#include "SignalJson.hpp"
#include "ThreadSafeQueue.hpp"

#include <atomic>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

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

    ThreadSafeQueue<can_frame> rawFrameQueue;
    ThreadSafeQueue<std::string> decodedMessageQueue;

    if (!mqtt.connect())
    {
        return 1;
    }

    if (!reader.open())
    {
        return 1;
    }

    std::atomic<bool> readerFailed{false};

    std::thread readerThread([&reader, &rawFrameQueue, &readerFailed]() {
        while (true)
        {
            can_frame frame;
            CanReader::ReadResult result = reader.readFrame(frame);

            if (result == CanReader::ReadResult::Frame)
            {
                rawFrameQueue.push(frame);
            }
            else if (result == CanReader::ReadResult::Error)
            {
                readerFailed = true;
                break;
            }
        }

        rawFrameQueue.shutdown();
    });

    std::thread decoderThread([&rawFrameQueue, &decodedMessageQueue, &decoder]() {
        while (std::optional<can_frame> frame = rawFrameQueue.waitAndPop())
        {
            std::vector<DecodedSignal> signals = decoder.decode(*frame);

            for (const DecodedSignal& signal : signals)
            {
                if (!signal.valid)
                {
                    continue;
                }

                decodedMessageQueue.push(toJson(signal));
            }
        }

        decodedMessageQueue.shutdown();
    });

    std::thread publisherThread([&decodedMessageQueue, &logger, &mqtt, &config]() {
        while (std::optional<std::string> message = decodedMessageQueue.waitAndPop())
        {
            std::cout << *message << std::endl;
            logger.log(*message);

            mqtt.publish(config.mqttTopic, *message);
        }
    });

    readerThread.join();
    decoderThread.join();
    publisherThread.join();

    return readerFailed ? 1 : 0;
}
