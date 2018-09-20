#include <fstream>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "WiFiAccessPointProtocol.hpp"


using namespace std;


WiFiAccessPointProtocol::WiFiAccessPointProtocol() :
    _haveExteriorLightsCommand(false)
,   _switchExteriorLightsOn(false)
,   _haveInteriorLightsCommand(false)
,   _switchInteriorLightsOn(false)
{
}


void WiFiAccessPointProtocol::initialize(const char* commandFilePath,
  const char* commandLockFilePath,const char* statusFilePath, const char* statusLockFilePath )
{
    _commandFilePath = commandFilePath;
    _commandLockFilePath = commandLockFilePath;
    _statusFilePath = statusFilePath;
    _statusLockFilePath = statusLockFilePath;
}


void WiFiAccessPointProtocol::updateState(GarageState state) {
    // create status lock file

    ofstream statusLock;
    statusLock.open(_statusLockFilePath);
    statusLock.close();

    // fill up status file

    ofstream statusFile;
    statusFile.open(_statusFilePath);
    statusFile << "door: ";

    switch (state.getMotorState()) {
    case RunningUp:
        statusFile << "opening";
        break;
    case RunningDown:
        statusFile << "closing";
        break;
    case Stopped:
        if(state.getStripePosition() == 0)
            statusFile << "closed";
        else if(state.getStripePosition() == state.getStripesCount() - 1)
            statusFile << "opened";
        else
            statusFile << "partial";
        break;
    case Blocked:
        statusFile << "blocked";
        break;
    }
    statusFile << endl;

     // check interior lights
    statusFile << "interior lights: ";
    if (state.getInteriorLights())
        statusFile << "on";
    else
        statusFile << "off";
    statusFile << endl;

    statusFile << "exterior lights: ";
    if(state.getExteriorLights())
        statusFile << "on";
    else
        statusFile << "off";
    statusFile << endl;

    statusFile.close();

    if (unlink(_statusLockFilePath)) {
        LOG_ERROR("Call to unlink('%s') failed: %s",
            _statusLockFilePath, strerror(errno));
        throw new WiFiAccessPointProtocolError();
    }
}

bool WiFiAccessPointProtocol::receiveCommand(GarageDriverCommand& command) {
    if (_haveExteriorLightsCommand) {
        command = _switchExteriorLightsOn ? SwitchExteriorLightsOn : SwitchExteriorLightsOff;
        _haveExteriorLightsCommand = false;
        return true;
    }

    if (_haveInteriorLightsCommand) {
        command = _switchInteriorLightsOn ? SwitchInteriorLightsOn : SwitchInteriorLightsOff;
        _haveInteriorLightsCommand = false;
        return true;
    }

    ifstream commandLockFile(_commandLockFilePath);
    if (commandLockFile.good())
        return false;
    commandLockFile.close();

    ifstream commandFile(_commandFilePath);
    if (!commandFile.good())
        return false;

    string doorCommand;
    commandFile >> doorCommand;
    if (doorCommand == "open")
        command = FullOpen;
    else
        command = FullClose;

    string switchExteriorLightsOn;
    commandFile >> switchExteriorLightsOn;
    _switchExteriorLightsOn = switchExteriorLightsOn == "on";
    _haveExteriorLightsCommand = true;

    string switchInteriorLightsOn;
    commandFile >> switchInteriorLightsOn;
    _switchInteriorLightsOn = switchInteriorLightsOn == "on";
    _haveInteriorLightsCommand = true;

    return true;
}
