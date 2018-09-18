#ifndef WIFIACCESSPOINTPROTOCOL_HPP
#define WIFIACCESSPOINTPROTOCOL_HPP


#include "GarageDriverCommand.hpp"
#include "GarageState.hpp"
#include "Logger.hpp"


class WiFiAccessPointProtocol {

public:
    void initialize(const char* path);

    void updateState(GarageState state);

    bool receiveCommand(GarageDriverCommand& command);

private:
    const char* _path;
};


#endif // !defined WIFIACCESSPOINTPROTOCOL_HPP
