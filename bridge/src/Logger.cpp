#include <iostream>
#include "Logger.hpp"
using namespace std;

static Logger* getInstance(){
    static Logger instance;
    return &instance;
}

    void Logger::initialize(LogLevel logLevel) {
	_level = logLevel;	
    }
    
    void Logger::logDebug(const char* message) {
    std::cout << "Test for Debug Logs" << endl;
    }
    
    void Logger::logInfo(const char* message) {
    std::cout << "Test for Info Logs" << endl;
    }
    
    void Logger::logWarning(const char* message) {
    std::cout << "Test for Warning Logs" << endl;
    }
    
    void Logger::logError(const char* message) {
    std::cout << "Test for Error Logs" << endl;
    }
    
    void Logger::setLogLevelFilter(LogLevel logLevel) {
    
    }

    void Logger::logMessage(LogLevel logLevel, const char* format, ...) {
	if (logLevel >= _level) {
	    va_list args;
	    va_start(args, format);
	    vprintf(format, args);
	    va_end(args);
	}
    }

#define LOG_DEBUG(format, ...) Logger::getInstance()->logMessage(Debug, "DEBUG:" format, __VA_ARGS__);
#define LOG_INFO(format, ...) Logger::getInstance()->logMessage(Info, "INFO:" format, __VA_ARGS__);
#define LOG_WARNING(format, ...) Logger::getInstance()->logMessage(Warning, "WARNING:" format, __VA_ARGS__);
#define LOG_ERROR(format, ...) Logger::getInstance()->logMessage(Error, "ERROR:" format, __VA_ARGS__);

int main(void){

LogLevel x = Debug;
Logger* logger = getInstance();
logger->logDebug("test");
/*
    if (x == Debug) {
    logger.logDebug("test");
    }
    else if (x == Info) {
        logInfo("test2");
    }
    else if (x == Warning) {
    logWarning("test3");
    }
    else if (x == Error) {
    logError("test4");
    }
    else {
        return 0;
    }
*/

}