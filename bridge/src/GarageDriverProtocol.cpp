#include <iostream>
#include <string>
#include <sstream>

#include "GarageDriverProtocol.hpp"


void GarageDriverProtocol::initialize(SerialPort port) {
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
    // string for testing 
    line = "abc, def,   ghi";

    // parse the line and set GarageStateParameters
    std::istringstream stream(line);
    size_t pos=-1;
    

    while (stream>>token){
        while ((pos=token.rfind(' ')) != std::string::npos) {
                  std::cout << token << '\n';
        }
    }
        

    return true;
}


void GarageDriverProtocol::sendCommand(GarageDriverCommand command) {


}
