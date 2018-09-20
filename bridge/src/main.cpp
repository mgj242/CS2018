#include <iostream>

#include "Config.hpp"
#include "Logger.hpp"
#include "SerialPort.hpp"
#include "GarageState.hpp"
#include "GarageDriverProtocol.hpp"
#include "WiFiAccessPointProtocol.hpp"
#include "GarageDriverCommand.hpp"


using namespace std;


int main() 
{
    //Initialization

    Config cfg;
    Logger::getInstance()->initialize(cfg.getLoggerLevel());

    SerialPort sp;
    sp.initialize(cfg.getSerialPortDevicePath(),
        cfg.getSerialPortBaudRate(),
        cfg.getSerialPortDataBitsCount(),
        cfg.getSerialPortStopBitsCount(),
        cfg.getSerialPortParity());

/*        cfg.getSmartHomeCentrallpAdress());
    cfg.getSmartHomeCentrallPort());
    cfg.getSmartHomeCentrallPath());
    cfg.getWifiApPath());*/

    GarageDriverProtocol gdp;
    gdp.initialize(&sp);

    WiFiAccessPointProtocol wapp;
    wapp.initialize(cfg.getWifiApStatusFilePath(),
        cfg.getWifiApStatusLockFilePath(),
        cfg.getWifiApCommandFilePath(),
        cfg.getWifiApCommandLockFilePath());

    while (1) {
        GarageState state;
        if (gdp.receiveState(state))
            wapp.updateState(state);
        GarageDriverCommand command;
        if (wapp.receiveCommand(command))
            gdp.sendCommand(command);
        //sp.pump();
    }

    return 0;
}
