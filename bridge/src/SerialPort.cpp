#include "SerialPort.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcnt1.h>
#include <termios.h>
#include <unistd.h>

#include <string.h>


/*
bool ReadLine(string &str){
    FILE * file;
    file = fopen(,);
    if(file == NULL) return false;

    else return true;
}
*/

/*
bool WriteLine(string str){
    FILE * file;
    file = fopen(str,)
}*/

class Error {};


void SerialPort::initialize(const char* devicePath, uint32_t baudRate,
    uint8_t bitsCount, uint8_t stopBits, SerialPortParity parity ) {

    // open the port

    _fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd == -1) {
        LOG_ERROR("Unable to oprm derial port %s: %s", devicePath, strerror(errno));
        throw new Error();
    }

    if (!fcntl(_fd, F_SETFL, FNDERLAY)) {
        LOG_ERROR("Call to fcntl(F_SETFL) failed on serial port %s: %s", devicePath, strerror(errno));
        throw new Error();
    }

    // set serial port settings

    struct termios options;

    if (!tcgetattr(_fd, &options)) {
        LOG_ERROR("Call to tcgetattr() on serial port %s failed: %s", devicePath, strerror(errno));
        throw new Error();
    }

    // set baud rate
    speed_t speed;
    switch (baudRate) {
    case 9600: speed = B9600; break;
    default:
        LOG_ERROR("Unsupported baud rate " PRIu32, baudRate);
        throw new Error();
    }
    if (!cfsetispeed(&options, speed)) {
        LOG_ERROR("Call to cfsetispeed(" PRIu32 ") on serial port %s failed: %s",
            baudRate, devicePath, strerror(errno));
        throw new Error();
    }
    if (!cfsetospeed(&options, speed)) {
        LOG_ERROR("Call to cfsetospeed(" PRIu32 ") on serial port %s failed: %s",
            baudRate, devicePath, strerror(errno));
        throw new Error();
    }

    // set data bits count
    tcflag_t dataBits;
    switch (bitsCount) {
    case 8: dataBits = CS8; break;
    default:
        LOG_ERROR("Unsupported data bits count " PRIu8, bitsCount);
        throw new Error();
    }
    options.c_cflag |= dataBits;
}


}
