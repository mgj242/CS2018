#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <string.h>
#include <unistd.h>

#include "Logger.hpp"
#include "SerialPort.hpp"


void SerialPort::initialize(const char* devicePath, uint32_t baudRate,
    uint8_t bitsCount, uint8_t stopBits, SerialPortParity parity) {

    // open the port

    _fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd == -1) {
        LOG_ERROR("Unable to oprm derial port %s: %s", devicePath, strerror(errno));
        throw new SerialPortError();
    }

    if (!fcntl(_fd, F_SETFL, FNDERLAY)) {
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
    options.c_cflag &= ~CNEW_RTSCTS;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // apply the settings
    if (!tcsetattr(_fd, TCSANOW, &options)) {
        LOG_ERROR("Call to tcsetattr() on serial port %s failed: %s",
            devicePath, strerror(errno));
        throw new SerialPortError();
    }
    LOG_DEBUG("Successfully configured serial port %s", devicePath);
}


// Interface


bool SerialPort::readLine(string& text)
{
}


bool SerialPort::writeLine(string text)
{
}
