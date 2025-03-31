#pragma once
#include <string>

enum LogLevel
{
    INFOLEVEL,
    WARNINGLEVEL,
    ERRORLEVEL,
};

const std::string LOG_FILENAME = "StrategyTimer.log";
void logmsg(std::string message, LogLevel level = LogLevel::INFOLEVEL);