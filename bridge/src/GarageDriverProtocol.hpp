#ifndef GARAGE_DRIVER_PROTOCOL_HPP
#define GARAGE_DRIVER_PROTOCOL_HPP

#include <string>

#include "GarageDriverCommand.hpp"
#include "GarageState.hpp"
#include "SerialPort.hpp"

class GarageDriverProtocol{

public:
    void initialize(SerialPort* port);

    /**
     * @desc Receive the state of garage driver from serial port.
     * @param state received garage state
     * @return true iff the state is available
     */
    bool recieveState(GarageState& state);

    void sendCommand(GarageDriverCommand command);

private:
    SerialPort* _port;
    void ParseCommand(std::string command);

};


#endif // !defined GARAGE_DRIVER_PROTOCOL_HPP
