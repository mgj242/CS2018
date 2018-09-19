#ifndef CONFIG_HPP
#define CONFIG_HPP


#include <cstdint>

#include "Logger.hpp"


/**
 * @desc Parity type used to configure the Linux serial port.
 */
enum SerialPortParity{
   Odd,
   Even,
   None
};


/**
 * @desc Class holding the application configuration.
 */
class Config {
public:
    LogLevel getLoggerLevel() const;

    const char* getSerialPortDevicePath() const;
    int32_t getSerialPortBaudRate() const;
    int8_t getSerialPortDataBitsCount() const;
    int8_t getSerialPortStopBitsCount() const; 
    SerialPortParity getSerialPortParity() const;

    const char* getSmartHomeCentrallpAddress() const;
    int32_t getSmartHomeCentrallPort() const;
    const char* getSmartHomeCentrallPath() const;

    const char* getWifiApPath() const;
};


#endif // !defined CONFIG_HPP
