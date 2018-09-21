#include <iostream>
#include <string>
#include <sstream>


#include "GarageDriverProtocol.hpp"
#include "Logger.hpp"

void GarageDriverProtocol::initialize(SerialPort* port) {
    // Save port to use later
    _port = port;
}


bool GarageDriverProtocol::recieveState(GarageState& state) {
    // read line from serial port
    std::string line;

/*
    if (!_port.readLine(line))
        return false;
*/
 line = "1 D5/1 B1 MS L3 l4 OK sdf";
 
    std::istringstream stream(line);

    // parse acknowledge ID

    uint32_t ackId = parseItem<uint32_t>(stream, line);

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

    // current stripe is numeric
    if (isalpha(stripeIxChar)) {
        switch (stripeIxChar) {
        case 'C': stripeIx = stripeCount - 1; break;
        case 'O': stripeIx = 0; break;
        default:
            LOG_ERROR("Unable to parse current strip in line: '%s' Char '%c; is niether 'C' nor 'O'  ",
                line.c_str(), stripeIxChar);
            throw new GarageDriverProtocolError();

        }
        // Current stripe is out of possible range
        
    } else{
        stripeIx = parseItem<uint8_t>(stream, line);
        
        if (stripeIx > stripeCount -1 || stripeIx < 0){
            LOG_ERROR("Current stripe is out of all possible stripes totalStripes: %i, currentStripe: %i line: %s",
            stripeCount, stripeIx, line.c_str());
            throw new GarageDriverProtocolError();
        }
     }

    // Discard 'B' 
    
    discardChar(stream, 'B', line);
    
    
    // Blockade present (0 no blockade 1 blockade present)
    
    bool blockadeIsPresent;
    uint8_t blockade = parseItem<uint8_t>(stream, line);
    
    switch(blockade)
    {
        case 0: blockadeIsPresent = false; break;
        case 1: blockadeIsPresent = true; break;
        default:
            LOG_ERROR("Unexpected number on present blockade %i is neither '0'(false) nor '1'(true) line: %s",
            blockade, line.c_str());
        throw new GarageDriverProtocolError();

    }
    
    // Discard 'M'
    
    discardChar(stream, 'M', line);

    // Parse Motor State (U)p (D)own (S)topped

    char motorState = parseItem<char>(stream, line);
    switch(motorState){
        case 'U': break;
        case 'D': break;
        case 'S': break;
        default:
        LOG_ERROR("Unexpected character on motorState %c is neither 'U' 'D' nor 'S' line: %s",
            motorState, line.c_str());
    }
    
    // indoorLights parsing
    
    bool isIndoorLightOn;
    uint8_t indoorLight = parseItem<uint8_t>(stream, line);
    
    switch(indoorLight)
    {
        case 0: isIndoorLightOn = false; break;
        case 1: isIndoorLightOn = true; break;
        default:
            LOG_ERROR("Unexpected number on interiorLigt %i is neither '0'(false) nor '1'(true) line: %s",
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
            LOG_ERROR("Unexpected number on exteriorLigt %i is neither '0'(false) nor '1'(true) line: %s",
            outDoorLight, line.c_str());
        throw new GarageDriverProtocolError();

    }
    
    
    
    std::cout << ackId << '\n'; 
    /*
    std::cout << expD << '\n';
    std::cout << lastStripe << '\n';
    std::cout << expSlash << '\n';
    std::cout << numOfStrips << '\n';
    std::cout << expB << '\n';
    std::cout << blockade << '\n';
    std::cout << expM << '\n';
    std::cout << motorState << '\n';
    std::cout << message << '\n';
*/

   
    return true;
}


void GarageDriverProtocol::sendCommand(GarageDriverCommand command) {

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
