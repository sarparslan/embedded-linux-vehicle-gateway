#pragma once

#include <string>

#include <mosquitto.h>

class MqttPublisher
{
public:
    MqttPublisher(const std::string& host, int port, const std::string& clientId);
    ~MqttPublisher();

    MqttPublisher(const MqttPublisher&) = delete;
    MqttPublisher& operator=(const MqttPublisher&) = delete;

    // Connects to the broker and starts the background network loop, which
    // handles keepalive pings and reconnects automatically if the link drops.
    bool connect();
    bool publish(const std::string& topic, const std::string& message);

private:
    std::string host_;
    int port_;
    std::string clientId_;
    mosquitto* client_;
    bool loopRunning_;
};
