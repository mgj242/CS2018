#pragma once


#include "common.h"

#include "CommChannel.h"
#include "SerialPort.h"


namespace Application {


class Controller
{
public:
    // Construction & destruction

    Controller() noexcept;
    ~Controller();

    // Interface

    bool connect(SerialPort&);
    bool update();
    void disconnect();

    bool fullDown();
    bool fullUp();
    bool stepDown();
    bool stepUp();
    bool stop();

    void simulateBlockage(bool);

    void switchIndoorLights(bool);
    void switchOutdoorLights(bool);


    // Accessors

    bool isBlocked() const noexcept { return m_motorState == Blocked; }

    bool isConnected() const noexcept { return m_commChannel.isConnected(); }
    bool isCommunicationError() const noexcept { return m_commChannel.isError(); }

    double getDoorPercentPosition() const noexcept { return abs((m_doorPosition * 100.) / m_doorPositionMax); }
    int  getDoorStripePosition() const noexcept; // 0 is at the very top ~ detected if fully closed
    bool setDoorStripePosition(int stripeIx);
    int getDoorStripesCount() const noexcept { return m_stripesCount; }
    bool setDoorStripesCount(int stripesCount);
    bool isFullyClosed() const noexcept { return m_doorPosition == 0; }
    bool isFullyOpened() const noexcept { return m_doorPosition == m_doorPositionMax; }

    bool isMotorRunning() const noexcept { return m_motorState != Stopped && m_motorState != Blocked; }
    MotorState getMotorState() const noexcept { return m_motorState; }

    int getSecondsToClose() const noexcept { return m_secondsToClose; }
    bool setSecondsToClose(int);
    int getSecondsToOpen() const noexcept { return m_secondsToOpen; }
    bool setSecondsToOpen(int);

    bool isIndoorLightsOn() const noexcept { return m_indoorLightsOn; }
    bool isOutdoorLightsOn() const noexcept { return m_outdoorLightsOn; }

private:
    // Types

    typedef uint16_t DoorPosition;


    // Constants

    enum {
        c_maxDoorStripes = 25,
        c_stepsPerStripe = 100
    };


    // Implementation

    int  _doorStripePosition(DoorPosition from) const noexcept;

    void _markMotorStart() noexcept;

    double _secondsSinceMotorStart() const noexcept;
    double _secondsToFullDoorClose() const noexcept;
    double _secondsToFullDoorOpen() const noexcept;
    double _secondsToStepDoorClose(int& nextStripe) const noexcept;
    double _secondsToStepDoorOpen(int& nextStripe) const noexcept;


    // Attributes

    CommChannel m_commChannel;

    int m_stripesCount;

    DoorPosition m_doorPosition; // 0. ~ fully closed, m_doorPositionMax ~ fully opened
    DoorPosition m_doorPositionMax;

    int m_secondsToClose;
    int m_secondsToOpen;

    MotorState m_motorState;
    TimePoint m_motorStartTime;
    DoorPosition m_doorStartPosition;

    bool m_indoorLightsOn;
    bool m_outdoorLightsOn;
};


} // namespace Application
