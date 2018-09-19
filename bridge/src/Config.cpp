#include "Config.hpp"


LogLevel Config::getLoggerLevel() const {
    return Debug;
}

const char* Config::getSerialPortDevicePath() const {
    return "/dev/ttyAMA0";
}

uint32_t Config::getSerialPortBaudRate() const {
    return 9600; 
}

SerialPortParity Config::getSerialPortParity() const{
    return Odd;
}

uint8_t Config::getSerialPortDataBitsCount() const {
    return 8;
}

SerialPortStopBits Config::getSerialPortStopBitsCount() const {
    return One;
}

const char* Config::getSmartHomeCentrallpAddress() const {
    return "address";
}

uint32_t Config::getSmartHomeCentrallPort() const {
    return 3000;
}
const char* Config::getSmartHomeCentrallPath() const {
    return "path";
}

const char* Config::getWifiApPath() const {
    return "Wifi_path";
}
