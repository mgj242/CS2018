#ifndef CONFIG_H
#define CONFIG_H


#include <cstdint>
enum SerialPortParity{
   Odd,
   Even,
   None
};

class Config {
public:
    // LogLevel getLoggerLevel() const;

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


#endif // !defined CONFIG_H
