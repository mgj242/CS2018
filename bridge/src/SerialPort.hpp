#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP


#include <cstdint>
#include <string>


/**
 * @desc Exception indicating an serial port error.
 */
class SerialPortError {
};


/**
 * @desc Parity type used to configure the Linux serial port.
 */
enum SerialPortParity{
   Odd,
   Even,
   None
};


/**
 * @desc Parity type used to configure the Linux serial port.
 */
enum SerialPortStopBits{
   One,
   Two
};


class SerialPort {
public:
    // Construction & destruction

    SerialPort();

    void initialize(const char* devicePath, uint32_t baudRate, uint8_t bitsCount,
        SerialPortStopBits stopBits, SerialPortParity parity);


    // Interface

    bool readLine(std::string& text);
    bool writeLine(std::string text);


private:
    // Construction & destruction

    SerialPort(SerialPort&); // disallow copying
    SerialPort& operator=(SerialPort&); // disallow copying


    // Constants

    enum { c_bufferSize = 1024 };


    // Attributes

    int _fd;

    uint8_t _buffer[c_bufferSize];
    uint16_t _currentIx;
};


#endif // !defined SERIAPORT_HPP
