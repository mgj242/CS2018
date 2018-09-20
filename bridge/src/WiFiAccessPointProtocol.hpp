#ifndef WIFIACCESSPOINTPROTOCOL_HPP
#define WIFIACCESSPOINTPROTOCOL_HPP


#include "GarageDriverCommand.hpp"
#include "GarageState.hpp"
#include "Logger.hpp"



class WiFiAccessPointProtocolError { };


class WiFiAccessPointProtocol {
public:
    WiFiAccessPointProtocol();

    void initialize(const char* commandFilePath,const char* commandLockFilePath,
        const char* statusFilePath, const char* statusLockFilePath );

    void updateState(GarageState state);

    bool receiveCommand(GarageDriverCommand& command);

private:
    const char* _commandFilePath;
    const char* _commandLockFilePath;
    const char* _statusFilePath;
    const char* _statusLockFilePath;

    bool _haveExteriorLightsCommand;
    bool _switchExteriorLightsOn;
    bool _haveInteriorLightsCommand;
    bool _switchInteriorLightsOn;
};


#endif // !defined WIFIACCESSPOINTPROTOCOL_HPP
