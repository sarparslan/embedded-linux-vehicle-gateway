#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <system_error>

Logger::Logger(const std::string& filePath)
{
    const std::filesystem::path parent = std::filesystem::path(filePath).parent_path();

    if (!parent.empty())
    {
        std::error_code error;
        std::filesystem::create_directories(parent, error);

        if (error)
        {
            std::cerr << "Failed to create log directory " << parent << ": "
                      << error.message() << std::endl;
        }
    }

    file_.open(filePath, std::ios::app);

    if (!file_.is_open())
    {
        std::cerr << "Failed to open log file " << filePath
                  << ", file logging is disabled" << std::endl;
    }
}

Logger::~Logger()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

void Logger::log(const std::string& message)
{
    if (!file_.is_open())
    {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime {};
    localtime_r(&time, &localTime);

    file_ << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
          << " "
          << message
          << std::endl;
}
