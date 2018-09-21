#ifndef GARAGEDRIVERPROTOCOL_HPP
#define GARAGEDRIVERPROTOCOL_HPP


#include <string>


#include "GarageDriverCommand.hpp"
#include "GarageState.hpp"
#include "SerialPort.hpp"


class GarageDriverProtocolError {};


class GarageDriverProtocol {
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
    // Implementation

    void discardChar(std::istringstream&, char, const std::string& line);

    template<typename Type>
    Type parseItem(std::istringstream&, const std::string& line);

    // Attributes

    SerialPort* _port;
};


#endif // !defined GARAGEDRIVERPROTOCOL_HPP
