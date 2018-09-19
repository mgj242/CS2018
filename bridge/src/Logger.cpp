#include <cstdio>

#include "Logger.hpp"


// Construction & destruction


Logger* Logger::getInstance(){
    static Logger instance;
    return &instance;
}

void Logger::initialize(LogLevel logLevel) {
    _level = logLevel;	
}


// Interface


void Logger::logMessage(LogLevel logLevel, const char* format, ...) {
    if (logLevel >= _level) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}


// Accessors


void Logger::setLogLevelFilter(LogLevel logLevel) {
    _level = logLevel;
}
