#ifndef LOGGER_HPP
#define LOGGER_HPP


#include <cstdarg>


enum LogLevel {
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4
};


class Logger {
public:
    // Construction & destruction

    void initialize(LogLevel logLevel);

    static Logger* getInstance();


    // Interface


    void logMessage(LogLevel logLevel, const char* message, ...);

    // Accessors

    void setLogLevelFilter(LogLevel logLevel);


private:
    // Attributes

    LogLevel _level;
};


#define LOG_DEBUG(format, ...) \
    Logger::getInstance()->logMessage(Debug, "DEBUG: " format, __VA_ARGS__);
#define LOG_INFO(format, ...) \
    Logger::getInstance()->logMessage(Info, "INFO: " format, __VA_ARGS__);
#define LOG_WARNING(format, ...) \
    Logger::getInstance()->logMessage(Warning, "WARNING: " format, __VA_ARGS__);
#define LOG_ERROR(format, ...) \
    Logger::getInstance()->logMessage(Error, "ERROR: " format, __VA_ARGS__);


#endif // !defined LOGGER_HPP
