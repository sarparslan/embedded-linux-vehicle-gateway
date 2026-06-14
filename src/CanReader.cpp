#include "CanReader.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <linux/can/raw.h>

namespace
{
    constexpr int kReceiveTimeoutMs = 200;
}

CanReader::CanReader(const std::string& interfaceName)
    : interfaceName_(interfaceName), socketFd_(-1)
{
}

CanReader::~CanReader()
{
    closeSocket();
}

void CanReader::closeSocket()
{
    if (socketFd_ >= 0)
    {
        close(socketFd_);
        socketFd_ = -1;
    }
}

bool CanReader::open()
{
    if (interfaceName_.empty() || interfaceName_.size() >= IFNAMSIZ)
    {
        std::cerr << "Invalid CAN interface name: '" << interfaceName_ << "'" << std::endl;
        return false;
    }

    socketFd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socketFd_ < 0)
    {
        std::cerr << "Failed to create CAN socket: " << std::strerror(errno) << std::endl;
        return false;
    }

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, interfaceName_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socketFd_, SIOCGIFINDEX, &ifr) < 0)
    {
        std::cerr << "Failed to find CAN interface " << interfaceName_ << ": "
                  << std::strerror(errno) << std::endl;
        closeSocket();
        return false;
    }

    struct sockaddr_can addr {};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socketFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        std::cerr << "Failed to bind CAN socket: " << std::strerror(errno) << std::endl;
        closeSocket();
        return false;
    }

    struct timeval timeout {};
    timeout.tv_sec = 0;
    timeout.tv_usec = kReceiveTimeoutMs * 1000;

    if (setsockopt(socketFd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        std::cerr << "Failed to set CAN receive timeout: " << std::strerror(errno) << std::endl;
        closeSocket();
        return false;
    }

    std::cout << "Listening on " << interfaceName_ << "..." << std::endl;
    return true;
}

CanReader::ReadResult CanReader::readFrame(can_frame& frame)
{
    ssize_t bytesRead = read(socketFd_, &frame, sizeof(frame));

    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return ReadResult::Timeout;
        }

        std::cerr << "Failed to read CAN frame: " << std::strerror(errno) << std::endl;
        return ReadResult::Error;
    }

    if (bytesRead != static_cast<ssize_t>(sizeof(frame)))
    {
        std::cerr << "Incomplete CAN frame (" << bytesRead << " bytes)" << std::endl;
        return ReadResult::Timeout;
    }

    return ReadResult::Frame;
}
