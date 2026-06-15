#pragma once

#include <fstream>
#include <string>

class Logger
{
public:
    // Creates the parent directory if needed. If the file cannot be opened,
    // an error is printed and log() becomes a no-op.
    explicit Logger(const std::string& filePath);
    ~Logger();

    void log(const std::string& message);

private:
    std::ofstream file_;
};
