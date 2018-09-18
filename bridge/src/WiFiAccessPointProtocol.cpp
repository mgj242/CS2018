#include <iostream>

#include "WiFiAccessPointProtocol.hpp"


void WiFiAccessPointProtocol::initialize(const char* path) {
    _path = path;
}

void  WiFiAccessPointProtocol::updateState(GarageState state) {

}

bool WiFiAccessPointProtocol::receiveCommand(GarageDriverCommand& command) {
    return false;
}