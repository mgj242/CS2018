#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP


#include <Config.hpp>
#include <cstdint>
#include <string>

class SerialPort{

public:

    void initialize(const char* SerialPortDevicePath, int32_t BaudRate, int8_t BitsCount, int8_t StopBits, SerialPortParity parity ) const;

    bool ReadLine(string str);
    bool WriteLine(string str);
}

#endif
