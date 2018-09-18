#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cinttypes>
#include <string.h>

#include "Logger.hpp"
#include "SerialPort.hpp"


// Construction & destruction


SerialPort::SerialPort() :
    _fd(-1)
,   _currentIx(0)
{
}


void SerialPort::initialize(const char* devicePath, uint32_t baudRate,
    uint8_t bitsCount, SerialPortStopBits stopBits, SerialPortParity parity) {

    if (_fd >= 0) {
        LOG_ERROR("Unable to initialize port %s - already initialized", devicePath);
        throw new SerialPortError();
    }

    // open the port

    _fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd == -1) {
        LOG_ERROR("Unable to open serial port %s: %s", devicePath, strerror(errno));
        throw new SerialPortError();
    }

    if (!fcntl(_fd, F_SETFL, FNDELAY)) {
        LOG_ERROR("Call to fcntl(F_SETFL) failed on serial port %s: %s", devicePath, strerror(errno));
        throw new SerialPortError();
    }

    // set serial port settings

    struct termios options;

    if (!tcgetattr(_fd, &options)) {
        LOG_ERROR("Call to tcgetattr() on serial port %s failed: %s", devicePath, strerror(errno));
        throw new SerialPortError();
    }

    // set baud rate
    speed_t speed;
    switch (baudRate) {
    case 9600: speed = B9600; break;
    default:
        LOG_ERROR("Unsupported baud rate " PRIu32, baudRate);
        throw new SerialPortError();
    }
    if (!cfsetispeed(&options, speed)) {
        LOG_ERROR("Call to cfsetispeed(" PRIu32 ") on serial port %s failed: %s",
            baudRate, devicePath, strerror(errno));
        throw new SerialPortError();
    }
    if (!cfsetospeed(&options, speed)) {
        LOG_ERROR("Call to cfsetospeed(" PRIu32 ") on serial port %s failed: %s",
            baudRate, devicePath, strerror(errno));
        throw new SerialPortError();
    }

    // set data bits count
    tcflag_t dataBits;
    switch (bitsCount) {
    case 8: dataBits = CS8; break;
    default:
        LOG_ERROR("Unsupported data bits count " PRIu8, bitsCount);
        throw new SerialPortError();
    }
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= dataBits;

    // set parity
    switch (parity) {
    case Odd:
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
    default:
        LOG_ERROR("Unsupported parity " PRIu32, (uint32_t)parity);
        throw new SerialPortError();
    }

    // set stop bits count
    switch (stopBits) {
    case One:
        options.c_cflag &= ~CSTOPB;
    default:
        LOG_ERROR("Unsupported stop bits count " PRIu32, (uint32_t)stopBits);
        throw new SerialPortError();
    }

    // set no flow control, and raw input and output
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // apply the settings
    if (!tcsetattr(_fd, TCSANOW, &options)) {
        LOG_ERROR("Call to tcsetattr() on serial port %s failed: %s",
            devicePath, strerror(errno));
        throw new SerialPortError();
    }


    _currentIx = 0;

    LOG_DEBUG("Successfully configured serial port %s", devicePath);
}


// Interface


bool SerialPort::readLine(std::string& text)
{
    uint8_t* currentPos = &_buffer[_currentIx];
    ssize_t bytesRead = read(_fd, currentPos, c_bufferSize - _currentIx);
    if (bytesRead == -1) {
        if (errno != EWOULDBLOCK) {
            LOG_ERROR("Call to read() on serial port failed: %s",
                strerror(errno));
            throw new SerialPortError();
        }
        return false;
    }

    for (uint16_t ix = bytesRead; ix > 0; ++_currentIx, --ix) {
        if (_buffer[_currentIx] == '\n') {
            if (_currentIx > 0)
                text = std::string(_buffer, &_buffer[_currentIx - 1]);
            else
                text.clear();
            if (ix > 0)
                memcpy(/*destination*/_buffer, /*source*/&_buffer[_currentIx+1],
                    /*count*/ix - 1);
            _currentIx = 0;
            return true;
        }
    }

    if (_currentIx >= c_bufferSize) {
        _buffer[c_bufferSize - 1] = '\0';
        LOG_ERROR("Serial port line '%s' too long", _buffer);
        throw new SerialPortError();
    }

    return false;
}


bool SerialPort::writeLine(std::string text)
{
    return false;
}
