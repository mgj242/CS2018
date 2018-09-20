#include <iostream>
#include <string>
#include <sstream>


#include "GarageDriverProtocol.hpp"


void GarageDriverProtocol::initialize(SerialPort* port) {
    // Save port to use later
    _port = port;
}


bool GarageDriverProtocol::recieveState(GarageState& state) {
    // read line from serial port
    std::string line;
    std::string token;

/*
    if (!_port.readLine(line))
        return false;
*/
 ParseCommand( "1 D5/1 B1 MS L3 l4 OK sdf");
   
    return true;
}

void GarageDriverProtocol::ParseCommand(const std::string& command){

   // string for testing 


    // parse the line and set GarageStateParameters
    std::istringstream stream(command);

    uint32_t ackId = parseItem<uint32_t>(stream);

    char expD;
    stream >> expD;
    
    char lastStripe;
    stream >> lastStripe;

    char expSlash;
    stream >> expSlash;     
     
    uint32_t numOfStrips;
    stream >> numOfStrips;    
    
    char expB;
    stream >> expB;

    bool blockade;
    stream >> blockade;

    char expM;
    stream >> expM;
    
    char motorState;
    stream >> motorState;
    
    char expL;
    stream >> expL;
    
    bool extLights = parseItem<bool>(stream);

    char expl;
    stream >> expl;

    bool intLights;
    stream >> intLights;
    
    std::string message;
    std::getline(stream, message);


    /*
    std::string message;
    stream >> message;
    */

    std::cout << ackId << '\n'; 
    std::cout << expD << '\n';
    std::cout << lastStripe << '\n';
    std::cout << expSlash << '\n';
    std::cout << numOfStrips << '\n';
    std::cout << expB << '\n';
    std::cout << blockade << '\n';
    std::cout << expM << '\n';
    std::cout << motorState << '\n';
    std::cout << message << '\n';



}

template<typename Type>
Type GarageDriverProtocol::parseItem(std::istringstream& stream) {
    Type result;
    stream >> result;
    if (!stream.good()) {
        throw new GarageDriverProtocolError();
    }
    return result;
}


void GarageDriverProtocol::sendCommand(GarageDriverCommand command) {


}
