#pragma once

#include <string>

#include <linux/can.h>

class CanReader
{
public:
    enum class ReadResult
    {
        Frame,
        Timeout,
        Error
    };

    explicit CanReader(const std::string& interfaceName);
    ~CanReader();

    CanReader(const CanReader&) = delete;
    CanReader& operator=(const CanReader&) = delete;

    bool open();

    // Blocks for at most the receive timeout set in open(), so the caller
    // can periodically check whether it should stop.
    ReadResult readFrame(can_frame& frame);

private:
    void closeSocket();

    std::string interfaceName_;
    int socketFd_;
};
