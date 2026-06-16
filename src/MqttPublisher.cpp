#include "MqttPublisher.hpp"

#include <iostream>

namespace
{
    constexpr int kKeepaliveSeconds = 60;
    constexpr unsigned int kReconnectDelayMinSeconds = 1;
    constexpr unsigned int kReconnectDelayMaxSeconds = 30;
}

MqttPublisher::MqttPublisher(const std::string& host, int port, const std::string& clientId)
    : host_(host), port_(port), clientId_(clientId), client_(nullptr), loopRunning_(false)
{
    mosquitto_lib_init();

    client_ = mosquitto_new(clientId_.c_str(), true, nullptr);

    if (client_ == nullptr)
    {
        std::cerr << "Failed to create MQTT client" << std::endl;
        return;
    }

    mosquitto_reconnect_delay_set(client_,
                                  kReconnectDelayMinSeconds,
                                  kReconnectDelayMaxSeconds,
                                  true);
}

MqttPublisher::~MqttPublisher()
{
    if (client_ != nullptr)
    {
        if (loopRunning_)
        {
            mosquitto_disconnect(client_);
            mosquitto_loop_stop(client_, false);
        }

        mosquitto_destroy(client_);
    }

    mosquitto_lib_cleanup();
}

bool MqttPublisher::connect()
{
    if (client_ == nullptr)
    {
        return false;
    }

    int result = mosquitto_connect(client_, host_.c_str(), port_, kKeepaliveSeconds);

    if (result != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to connect to MQTT broker: "
                  << mosquitto_strerror(result)
                  << std::endl;
        return false;
    }

    result = mosquitto_loop_start(client_);

    if (result != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to start MQTT network loop: "
                  << mosquitto_strerror(result)
                  << std::endl;
        return false;
    }

    loopRunning_ = true;

    std::cout << "Connected to MQTT broker at "
              << host_ << ":" << port_
              << std::endl;

    return true;
}

bool MqttPublisher::publish(const std::string& topic, const std::string& message)
{
    if (client_ == nullptr)
    {
        return false;
    }

    int result = mosquitto_publish(
        client_,
        nullptr,
        topic.c_str(),
        static_cast<int>(message.size()),
        message.c_str(),
        0,
        false
    );

    if (result != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to publish MQTT message: "
                  << mosquitto_strerror(result)
                  << std::endl;
        return false;
    }

    return true;
}
