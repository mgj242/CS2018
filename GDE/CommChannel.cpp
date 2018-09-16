#include "stdafx.h"

#include "common.h"
#include "CommChannel.h"
#include "Log.h"


namespace Application {


// Constants


const double CommChannel::c_secondsSinceReceiveAlert = 5.;


// Construction & destruction


CommChannel::CommChannel() noexcept :
    m_serialPort(nullptr)
,   m_outMessageId(0)
,   m_isConnected(false)
,   m_isError(false)
,   m_isAcknowledged(false)
{
}


CommChannel::~CommChannel()
{
}


// Interface


bool CommChannel::connect(SerialPort& serialPort)
{
    if (m_isConnected) {
        APP_LOG_ERROR(_T("Already connected"));
        return false;
    }
    m_serialPort = &serialPort;
    m_outMessageId = 0;
    m_lastAlertReceivedLineTime = m_lastReceivedLineTime = std::chrono::high_resolution_clock::now();
    m_isAcknowledged = true;
    m_isConnected = true;
    m_isError = false;

    return true;
}

void CommChannel::disconnect()
{
    if (!m_isConnected) {
        APP_LOG_ERROR(_T("Not connected"));
        return;
    }
    m_isConnected = false;
}

bool CommChannel::pump()
{
    if (!m_isConnected) {
        APP_LOG_ERROR(_T("Not connected"));
        return false;
    }
    ATLASSERT(m_serialPort != nullptr);
    return m_serialPort->pump();
}


bool CommChannel::receiveCommand(Command& result)
{
    ATLASSERT(m_serialPort != nullptr);

    // read line from the COM port, if available
    const auto now = std::chrono::high_resolution_clock::now();
    const double secondsFromLastLine = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastReceivedLineTime).count() / 1000.;
    if (secondsFromLastLine >= c_secondsSinceReceiveAlert) {
        const double secondsFromLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_lastAlertReceivedLineTime).count() / 1000.;
        if (secondsFromLastAlert >= c_secondsSinceReceiveAlert) {
            if (m_outMessageId > 1)
                APP_LOG_DEBUG(_T("Last valid serial port line received %.02f seconds ago with ACK ID %u"),
                    secondsFromLastLine, m_outMessageId - 1);
            else
                APP_LOG_DEBUG(_T("No valid serial port line received for %.02f seconds"),
                    secondsFromLastLine);
            m_lastAlertReceivedLineTime = std::chrono::high_resolution_clock::now();
        }
    }

    String line;
    if (!m_serialPort->readLine(std::move(line)))
        return false;
    if (line.empty()) {
        result = Nop;
        return true;
    }
    m_lastReceivedLineTime = std::chrono::high_resolution_clock::now();
    m_isError = false;

    // determine whether line contains only ACK, or a command
    IStringStream buf(line);
    buf >> std::skipws;

    const IStringStream::traits_type::int_type nextChar = buf.peek();
    if (nextChar == IStringStream::traits_type::eof()) {
        // empty line
        result = Nop;
        return true;
    }
    if (buf.fail()) {
        APP_LOG_ERROR(_T("Failed to parse COM port input line \"%s\""), line.c_str());
        m_isError = true;
        return false;
    }

    if (nextChar == 'A') {
        // skip the A character
        TCHAR a;
        buf >> a;
        ATLASSERT(a == nextChar);

        // parse and verify ACK ID
        if (!_parseAckID(line, buf))
            return false;

        result = Nop;
    } else {
        // parse and verify ACK ID
        if (!_parseAckID(line, buf))
            return false;

        // parse the command
        TCHAR command;
        buf >> command;
        if (buf.fail()) {
            APP_LOG_ERROR(_T("Failed to parse command from COM port input line \"%s\""), line.c_str());
            m_isError = true;
            return false;
        }

        switch (command) {
        case _T('C'):
            result = FullClose;
            APP_LOG_DEBUG(_T("Received full close command 'C'"));
            break;
        case _T('c'):
            result = CloseToNextStripe;
            APP_LOG_DEBUG(_T("Received close to next stripe command 'c'"));
            break;
        case _T('O'):
            result = FullOpen;
            APP_LOG_DEBUG(_T("Received full open command 'O'"));
            break;
        case _T('o'):
            result = OpenToNextStripe;
            APP_LOG_DEBUG(_T("Received open to next stripe command 'o'"));
            break;
        case _T('S'):
            result = StopMotor;
            APP_LOG_DEBUG(_T("Received stop motor command 'S'"));
            break;
        case _T('L'): { // switch the indoor lights
            bool subCommand;
            _parseBoolean(line, buf, subCommand);
            result = subCommand ? IndoorLightsOn : IndoorLightsOff;
            APP_LOG_DEBUG(_T("Received switch %s indoor lights command 'L'"), subCommand ? _T("on") : _T("off"));
            break; }
        case _T('l'): { // switch the outdoor lights
            bool subCommand;
            _parseBoolean(line, buf, subCommand);
            result = subCommand ? OutdoorLightsOn : OutoorLightsOff;
            APP_LOG_DEBUG(_T("Received switch %s outdoor lights command 'l'"), subCommand ? _T("on") : _T("off"));
            break; }
        default:
            APP_LOG_ERROR(_T("Unknown command '%c' parsed from COM port input line \"%s\""), command, line.c_str());
            m_isError = true;
            return false;
        }
    }

    // check that there is nothing left on the line
    String lineEnd;
    getline(buf, lineEnd);
    if (!lineEnd.empty()) {
        APP_LOG_ERROR(_T("Spurious text \"%s\" at the end of COM port input line \"%s\""), lineEnd.c_str(), line.c_str());
        m_isError = true;
        return true;
    }

    return true;
}

bool CommChannel::updateStatus(int curDoorStripe, int stripesCount, MotorState motorState,
    bool indoorLightsOn, bool outdoorLightsOn, const TCHAR* message)
{
    ATLASSERT(m_serialPort != nullptr);

    if (!m_isAcknowledged)
        return true;
    m_isAcknowledged = false;

    OStringStream buffer;

    buffer << ++m_outMessageId << _T(" D");
    if (curDoorStripe == 0)
        buffer << _T('C');
    else if (curDoorStripe == stripesCount)
        buffer << _T('O');
    else
        buffer << curDoorStripe;

    buffer << _T('/') << stripesCount
        << _T(" B") << (motorState == Blocked ? _T('1') : _T('0'))
        << _T(" M") << (motorState == RunningDown ? _T('D') : motorState == RunningUp ? _T('U') : _T('S'))
        << _T(" L") << (indoorLightsOn ? _T('1') : _T('0'))
        << _T(" l") << (outdoorLightsOn ? _T('1') : _T('0'))
        << _T(' ') << message;
    return m_serialPort->writeLine(buffer.str());
}


// Implementation


bool CommChannel::_parseAckID(const String& line, IStringStream& buf)
{
    MessageID ackId;
    buf >> ackId;
    if (buf.fail()) {
        APP_LOG_ERROR(_T("Failed to parse acknowledge ID from COM port input line \"%s\""), line.c_str());
        m_isError = true;
        return false;
    }
    if (m_outMessageId == ackId)
        m_isAcknowledged = true;
    else if (ackId > m_outMessageId) {
        APP_LOG_ERROR(_T("Future acknowledge ID %u received in line \"%s\", while expecting %u"), ackId, line.c_str(), m_outMessageId);
        m_isError = true;
        return false;
    } else {
        APP_LOG_ERROR(_T("Past acknowledge ID %u received in line \"%s\", while expecting %u"), ackId, line.c_str(), m_outMessageId);
        m_isError = true;
        return false;
    }
    return true;
}

bool CommChannel::_parseBoolean(const String& line, IStringStream& buf, bool& result)
{
    TCHAR subCommand;
    buf >> subCommand;
    switch (subCommand) {
    case '0': result = false; break;
    case '1': result = true; break;
    default:
        APP_LOG_ERROR(_T("Expected boolean subcommand value in COM port line \"%s\" instead of '%c', after \"%s\""),
            line.c_str(), subCommand, buf.str().c_str());
        m_isError = true;
        return false;
    }
    if (buf.fail()) {
        APP_LOG_ERROR(_T("Failed to parse boolean subcommand from COM port line \"%s\", after \"%s\""), line.c_str(), buf.str().c_str());
        m_isError = true;
        return false;
    }
    return true;
}


} // namespace Application
