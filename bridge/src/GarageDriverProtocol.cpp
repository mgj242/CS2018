#include <cinttypes>
#include <string>
#include <sstream>


#include "GarageDriverProtocol.hpp"
#include "Logger.hpp"
#include "SerialPort.hpp"


void GarageDriverProtocol::initialize(SerialPort* port) {
    // Save port to use later
    _port = port;
}


bool GarageDriverProtocol::receiveState(GarageState& state) {

    // read line from serial port and parse it


    // read line from serial port

    std::string line;

    if (!_port->readLine(line))
        return false;

    std::istringstream stream(line);

    // parse acknowledge ID

    GarageDriverProtocol::_ackId = parseItem<uint32_t>(stream, line);
    GarageDriverProtocol::_waitingForAcknowledge = true;

    // discard 'D'

    discardChar(stream, 'D', line);

    // parse current stripe index

    char stripeIxChar = stream.peek();

    // discard '/'

    discardChar(stream, '/', line);

    // parse stripes count

    uint8_t stripeCount = parseItem<uint8_t>(stream, line);

    // compute stripe index

    uint8_t stripeIx;

    // current stripe is not numeric
    if (isalpha(stripeIxChar)) {
        switch (stripeIxChar) {
        case 'C': stripeIx = 0; break;
        case 'O': stripeIx = stripeCount - 1; break;
        default:
            LOG_ERROR("Unable to parse current stripe '%c' in line '%s'", stripeIxChar, line.c_str());
            throw new GarageDriverProtocolError();
        }
    } else {
        stripeIx = parseItem<uint8_t>(stream, line);

        if (stripeIx >= stripeCount) {
            LOG_ERROR("Current stripe " PRIu8 " is out of range, while parsing line '%s'",
                stripeIx, line.c_str());
            throw new GarageDriverProtocolError();
        }
     }

    // Discard 'B' 
    
    discardChar(stream, 'B', line);
    
    // Blockade present (0 no blockade 1 blockade present)
    
    // Motor state representing running up down stoped or blocked
    MotorState motorState;
    
    uint8_t blockade = parseItem<uint8_t>(stream, line);
    
    switch (blockade) {
        case 0: /* don't have to set motor state */ break;
        case 1: motorState = Blocked; break;
        default:
            LOG_ERROR("Unexpected number on present blockade " PRIu8 " is neither '0'(false) nor '1'(true) in line: '%s'",
                blockade, line.c_str());
            throw new GarageDriverProtocolError();
    }

    // Discard 'M'
    
    discardChar(stream, 'M', line);

    // Parse Motor State (U)p (D)own (S)topped

    char motorStateSymbol = parseItem<char>(stream, line);

    if (motorState != Blocked) {
        switch (motorStateSymbol){
            case 'U': motorState = RunningUp; break;
            case 'D': motorState = RunningDown; break;
            case 'S': motorState = Stopped; break;
            default:
            LOG_ERROR("Unexpected character on motorState %i is neither 'U' 'D' nor 'S' line: %s",
                motorState, line.c_str());
        }
    }

    // indoor lights parsing
    
    bool isIndoorLightOn;
    uint8_t indoorLight = parseItem<uint8_t>(stream, line);
    
    switch(indoorLight)
    {
        case 0: isIndoorLightOn = false; break;
        case 1: isIndoorLightOn = true; break;
        default:
            LOG_ERROR("Unexpected number on interiorLigt " PRIu8  " is neither '0'(false) nor '1'(true) line: %s",
                indoorLight, line.c_str());
            throw new GarageDriverProtocolError();
    }

    // discard 'l'
    discardChar(stream, 'l', line);

    bool isOutDoorLightOn;
    uint8_t outDoorLight = parseItem<uint8_t>(stream, line);
    
    switch(outDoorLight)
    {
        case 0: isOutDoorLightOn = true; break;
        case 1: isOutDoorLightOn = false; break;
        default:
            LOG_ERROR("Unexpected number on exteriorLigt" PRIu8  " is neither '0'(false) nor '1'(true) line: %s",
                outDoorLight, line.c_str());
            throw new GarageDriverProtocolError();
    }

    // use parsed attributes to set state of Garage
    
    state.setStripePosition(stripeIx);
    state.setStripesCount(stripeCount);
    state.setMotorState(motorState);
    state.setInteriorLights(isIndoorLightOn);
    state.setExteriorLights(isOutDoorLightOn);

    return true;
}


void GarageDriverProtocol::pump(){
    if (!_waitingForAcknowledge)
        return;

    std::ostringstream stream;
    
    stream << "A " << _ackId;

    _waitingForAcknowledge = !_port->writeLine(stream.str());
}


void GarageDriverProtocol::sendCommand(GarageDriverCommand command) {
    std::ostringstream stream;
    switch (command) {
    case FullOpen:
        stream << 'O';
        break;
    case FullClose:
        stream << 'C';
        break;
    case OpenToNextStrip:
        stream << 'o';
        break;
    case CloseToNextStrip:
        stream << 'c';
        break;
    case SwitchInteriorLightsOn:
        stream << 'L' << '1';
        break;
    case SwitchInteriorLightsOff:
        stream << 'L' << '0';
        break;
    case SwitchExteriorLightsOn:
        stream << 'l' << '1';
        break;
    case SwitchExteriorLightsOff:
        stream << 'l' << '0';
        break;
	default:
	    LOG_ERROR("Unexpected command %i", command);
            throw new GarageDriverProtocolError();
	    break;
    }


	// Try to send command
    while (_port->writeLine(stream.str())) {
        _port->pump();
    }
}


// Implementation


void GarageDriverProtocol::discardChar(std::istringstream& stream, char expected, const std::string& line) {
    char actual = parseItem<char>(stream, line);
    if (expected != actual) {
        LOG_ERROR("Expected character '%c' instead of '%c' while parsing garage door controller line '%s'",
            expected, actual, line.c_str());
        throw new GarageDriverProtocolError();
    }
}

template<typename Type>
Type GarageDriverProtocol::parseItem(std::istringstream& stream, const std::string& line) {
    Type result;
    stream >> result;
    if (!stream.good()) {
        LOG_ERROR("Unable to parse garage door controller line '%s'", line.c_str());
        throw new GarageDriverProtocolError();
    }
    return result;
}

