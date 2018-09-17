#ifndef LOGGER_H
#define LOGGER_H

#include <cstdarg>

enum LogLevel {
Debug = 1,
Info = 2,
Warning = 3,
Error = 4
};

class Logger {

public:

    static Logger* getInstance();
    void initialize(LogLevel logLevel);
    void logDebug(const char* message);
    void logInfo(const char* message);
    void logWarning(const char* message);
    void logError(const char* message);
    void setLogLevelFilter(LogLevel logLevel);

private:
    void logMessage(LogLevel logLevel, const char* message, ...);
    LogLevel _level;

};

#endif