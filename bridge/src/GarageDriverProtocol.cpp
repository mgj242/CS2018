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
 ParseCommand( "1 D5/105 B1 MS L3 l4 OK");
   
    return true;
}

void GarageDriverProtocol::ParseCommand(std::string command){

   // string for testing 


    // parse the line and set GarageStateParameters
    std::istringstream stream(command);

    uint32_t ackId;
    stream >> ackId;

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

    

    std::cout << ackId << '\n' ; 
    std::cout << expD << '\n' ;
    std::cout << lastStripe << '\n' ;
    std::cout << numOfStrips << '\n' ;
    std::cout << expB << '\n' ;
    std::cout << blockade << '\n' ;

}


void GarageDriverProtocol::sendCommand(GarageDriverCommand command) {


}
