#include "Config.hpp"



const char* Config::getSerialPortDevicePath() const {
    return "/dev/ttyAMA0";
}


int32_t Config::getSerialPortBaudRate() const {

	return 9600; 
}

SerialPortParity Config::getSerialPortParity() const{
   return Odd;
}

int8_t Config::getSerialPortDataBitsCount() const {
	return 8;
}

int8_t Config::getSerialPortStopBitsCount() const 
{
	return 1;
}

const char* Config::getSmartHomeCentrallpAddress() const {
	return "address";
}

int32_t Config::getSmartHomeCentrallPort() const{
	return 3000;
}
const char* Config::getSmartHomeCentrallPath() const{
	return "path";
}

const char* Config::getWifiApPath() const{
	return "Wifi_path";
}
