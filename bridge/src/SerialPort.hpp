#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP


#include <Config.hpp>
#include <cstdint>

class SerialPort{

public:

    void initialize(const char* devicePath, int32_t baudRate, int8_t bitsCount, int8_t stopBits, SerialPortParity parity ) ;

}





#endif
