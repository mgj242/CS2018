#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP


#include <cstdint>
#include <string>

#include <Config.hpp>


/**
 * @desc Exception indicating an serial port error.
 */
class SerialPortError {
};


class SerialPort {
public:
    // Construction & destruction

    void initialize(const char* devicePath, uint32_t baudRate, uint8_t bitsCount,
        uint8_t stopBits, SerialPortParity parity);


    // Interface

    bool readLine(std::string& text);
    bool writeLine(std::string text);


private:
    // Attributes

    int _fd;
};


#endif // !defined SERIAPORT_HPP
