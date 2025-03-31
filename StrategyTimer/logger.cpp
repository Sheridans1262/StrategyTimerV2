#define _CRT_SECURE_NO_WARNINGS
#include "logger.h"
#include <fstream>
#include <chrono>

void logmsg(std::string message, LogLevel level)
{
    std::ofstream file(LOG_FILENAME, std::ios::app);
    if (file.is_open())
    {
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        file << std::put_time(std::localtime(&time), "%F %T - ");
        file << level << ": " << message << "\n";
        file.close();
    }
}