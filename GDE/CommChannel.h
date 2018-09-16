#pragma once


#include "SerialPort.h"


namespace Application {


class CommChannel
{
public:
    // Construction & destruction

    CommChannel() noexcept;
    ~CommChannel();


    // Interface

    bool connect(SerialPort& serialPort);
    void disconnect();

    bool pump();
    bool receiveCommand(Command&);
    bool updateStatus(int curDoorStripe, int stripesCount, MotorState motorState,
        bool indoorLightsOn, bool outdoorLightsOn, const TCHAR* message);


    // Accessors

    bool isConnected() const noexcept { return m_isConnected; }
    bool isError() const noexcept { return m_isError; }

private:
    // Types

    typedef uint32_t MessageID;

    // Constants

    static const double c_secondsSinceReceiveAlert;


    // Implementation

    bool _parseAckID(const String& line, IStringStream&);
    bool _parseBoolean(const String& line, IStringStream&, bool& result);

    // Attributes

    SerialPort* m_serialPort;

    MessageID m_outMessageId;

    TimePoint m_lastReceivedLineTime;
    TimePoint m_lastAlertReceivedLineTime;

    bool m_isConnected;
    bool m_isError;
    bool m_isAcknowledged;
};


} // namespace Application