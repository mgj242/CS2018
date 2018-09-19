#ifndef CONFIG_HPP
#define CONFIG_HPP


#include <cstdint>

#include "Logger.hpp"
#include "SerialPort.hpp"


/**
 * @desc Class holding the application configuration.
 */
class Config {
public:
    LogLevel getLoggerLevel() const;

    const char* getSerialPortDevicePath() const;
    uint32_t getSerialPortBaudRate() const;
    uint8_t getSerialPortDataBitsCount() const;
    SerialPortStopBits getSerialPortStopBitsCount() const; 
    SerialPortParity getSerialPortParity() const;

    const char* getSmartHomeCentrallpAddress() const;
    uint32_t getSmartHomeCentrallPort() const;
    const char* getSmartHomeCentrallPath() const;

    const char* getWifiApPath() const;
};


#endif // !defined CONFIG_HPP
