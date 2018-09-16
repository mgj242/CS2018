#include "stdafx.h"

#include "Controller.h"
#include "Log.h"


namespace Application {


// Construction & destruction


Controller::Controller() noexcept :
    m_stripesCount(6)
,   m_doorPosition(0/*closed*/)
,   m_doorPositionMax(0)
,   m_secondsToClose(8)
,   m_secondsToOpen(6)
,   m_motorState(Stopped)
,   m_doorStartPosition(0/*closed*/)
,   m_indoorLightsOn(false)
,   m_outdoorLightsOn(false)
{
}

Controller::~Controller()
{
}


// Interface


bool Controller::connect(SerialPort& serialPort)
{
    if (m_commChannel.isConnected()) {
        APP_LOG_WARNING(_T("Already connected"));
        return false;
    }
    return m_commChannel.connect(serialPort);
}

void Controller::disconnect()
{
    stop();
    if (m_commChannel.isConnected())
        m_commChannel.disconnect();
}


bool Controller::update()
{
    if (!isConnected())
        return true;

    // receive and execute command from COM port, if there is any
    Command command;
    if (m_commChannel.receiveCommand(command)) {
        switch (command) {
        case Nop: break;
        case FullClose:         fullDown(); break;
        case CloseToNextStripe: stepDown(); break;
        case FullOpen:          fullUp(); break;
        case OpenToNextStripe:  stepUp(); break;
        case StopMotor:         stop(); break;
        case IndoorLightsOn:    switchIndoorLights(true); break;
        case IndoorLightsOff:   switchIndoorLights(false); break;
        case OutdoorLightsOn:   switchOutdoorLights(true); break;
        case OutoorLightsOff:   switchOutdoorLights(false); break;
        default:
            ATLASSERT(false/*unexpected*/);
        }
    }

    // move doors
    StringStream messageBuffer;
    messageBuffer << std::fixed << std::setprecision(2);

    switch (m_motorState) {
    case Stopped: // nop
    case Blocked: // nop
        break;
    case RunningDown: {
        const double secondsToClose = _secondsToFullDoorClose();
        const double timePassed = _secondsSinceMotorStart();
        if (timePassed < secondsToClose) {
            m_doorPosition = static_cast<DoorPosition>(m_doorStartPosition * (1. - timePassed/secondsToClose));
            messageBuffer << "Closing " << getDoorPercentPosition() << "%";
        } else {
            m_doorPosition = 0;
            m_motorState = Stopped;
            messageBuffer << "Fully closed";
            APP_LOG_INFO(_T("Door fully closed"));
        }
        break; }
    case RunningUp: {
        const double secondsToOpen = _secondsToFullDoorOpen();
        const double timePassed = _secondsSinceMotorStart();
        if (timePassed < secondsToOpen) {
            m_doorPosition = static_cast<DoorPosition>(m_doorStartPosition +
                (timePassed / secondsToOpen)*(m_doorPositionMax - m_doorStartPosition));
            messageBuffer << "Opening " << getDoorPercentPosition() << "%";
        } else {
            m_doorPosition = m_doorPositionMax;
            m_motorState = Stopped;
            messageBuffer << "Fully opened";
            APP_LOG_INFO(_T("Door fully opened"));
        }
        break; }
    case SteppingDown: {
        int nextStripe;
        const double secondsToClose = _secondsToStepDoorClose(nextStripe);
        const double timePassed = _secondsSinceMotorStart();
        const DoorPosition nextStripePos = nextStripe * c_stepsPerStripe;
        ATLASSERT(nextStripePos <= m_doorStartPosition);
        if (timePassed < secondsToClose) {
            m_doorPosition = static_cast<DoorPosition>(m_doorStartPosition -
                (timePassed / secondsToClose)*(m_doorStartPosition - nextStripePos));
            messageBuffer << "Closing " << getDoorPercentPosition() << "%";
        } else {
            m_doorPosition = nextStripePos;
            m_motorState = Stopped;
            messageBuffer << "Closed to stripe #%u" << nextStripe;
            APP_LOG_INFO(_T("Door closed to stripe #%u"), nextStripe);
        }
        break; }
    case SteppingUp: {
        int nextStripe;
        const double secondsToOpen = _secondsToStepDoorOpen(nextStripe);
        const double timePassed = _secondsSinceMotorStart();
        const DoorPosition nextStripePos = nextStripe * c_stepsPerStripe;
        ATLASSERT(nextStripePos >= m_doorStartPosition);
        if (timePassed < secondsToOpen) {
            m_doorPosition = static_cast<DoorPosition>(m_doorStartPosition +
                (timePassed / secondsToOpen)*(nextStripePos - m_doorStartPosition));
            messageBuffer << "Opening " << getDoorPercentPosition() << "%";
        } else {
            m_doorPosition = nextStripePos;
            m_motorState = Stopped;
            messageBuffer << "Opened to stripe #%u" << nextStripe;
            APP_LOG_INFO(_T("Door opened to stripe #%u"), nextStripe);
        }
        break; }
    default:
        ATLASSERT(false/*unexpected*/);
        messageBuffer << "Unexpected motor state " << m_motorState;
        APP_LOG_ERROR(_T("Unexpected motor state %u"), m_motorState);
    }

    if (!m_commChannel.pump())
        return false;
    return m_commChannel.updateStatus(getDoorStripePosition(), m_stripesCount, m_motorState,
        m_indoorLightsOn, m_outdoorLightsOn, messageBuffer.str().c_str());
}


bool Controller::fullDown()
{
    if (!isConnected()) {
        APP_LOG_ERROR(_T("Unable to perform full closing if not connected"));
        return false;
    }
    if (m_motorState != Blocked && m_motorState != RunningDown) {
        m_motorState = RunningDown;
        _markMotorStart();
        APP_LOG_INFO(_T("Motor started running down, will finish in %.02f seconds"), _secondsToFullDoorClose());
    }
    return true;
}

bool Controller::fullUp()
{
    if (!isConnected()) {
        APP_LOG_ERROR(_T("Unable to perform full opening if not connected"));
        return false;
    }
    if (m_motorState != Blocked && m_motorState != RunningUp) {
        m_motorState = RunningUp;
        _markMotorStart();
        APP_LOG_INFO(_T("Motor started running up, will finish in %.02f seconds"), _secondsToFullDoorOpen());
    }
    return true;
}

bool Controller::stepDown()
{
    if (!isConnected()) {
        APP_LOG_ERROR(_T("Unable to step down if not connected"));
        return false;
    }
    if (m_motorState != Blocked && m_motorState != SteppingDown) {
        m_motorState = SteppingDown;
        _markMotorStart();
        int nextStripe;
        const double secondsToClose = _secondsToStepDoorClose(nextStripe);
        APP_LOG_INFO(_T("Motor started stepping down, will finish in %.02f seconds on stripe #%u"), secondsToClose, nextStripe);
    }
    return true;
}

bool Controller::stepUp()
{
    if (!isConnected()) {
        APP_LOG_ERROR(_T("Unable to step up if not connected"));
        return false;
    }
    if (m_motorState != Blocked && m_motorState != SteppingUp) {
        m_motorState = SteppingUp;
        _markMotorStart();
        int nextStripe;
        const double secondsToOpen = _secondsToStepDoorOpen(nextStripe);
        APP_LOG_INFO(_T("Motor started stepping up, will finish in %.02f seconds on stripe #%u"), secondsToOpen, nextStripe);
    }
    return true;
}

bool Controller::stop()
{
    if (!isConnected()) {
        APP_LOG_ERROR(_T("Unable to stop the motor if not connected"));
        return false;
    }
    if (m_motorState != Blocked) {
        m_motorState = Stopped;
        APP_LOG_INFO(_T("Motor stopped near stripe #%u"), getDoorStripePosition());
    }
    return true;
}


void Controller::simulateBlockage(bool isBlocked)
{
    if (isBlocked) {
        m_motorState = Blocked;
        APP_LOG_INFO(_T("Motor blocked near stripe #%u"), getDoorStripePosition());
    } else if (m_motorState == Blocked) {
        m_motorState = Stopped;
        APP_LOG_INFO(_T("Motor unblocked near stripe #%u"), getDoorStripePosition());
    }
}


void Controller::switchIndoorLights(bool value)
{
    if (m_indoorLightsOn != value) {
        m_indoorLightsOn = value;
        APP_LOG_INFO(_T("Indoor light switched %s"), value ? _T("on") : _T("off"));
    }
}

void Controller::switchOutdoorLights(bool value)
{
    if (m_outdoorLightsOn != value) {
        m_outdoorLightsOn = value;
        APP_LOG_INFO(_T("Outdoor light switched %s"), value ? _T("on") : _T("off"));
    }
}


// Accessors


int Controller::getDoorStripePosition() const noexcept
{
    return _doorStripePosition(m_doorPosition);
}

bool Controller::setDoorStripePosition(int stripeIx)
{
    if (stripeIx < 0) {
        APP_LOG_ERROR(_T("Invalid stripe index %d"), stripeIx);
        return false;
    }
    ATLENSURE(stripeIx < m_stripesCount);
    if (isConnected()) {
        APP_LOG_ERROR(_T("Unable to set door stripe position to %d while connected"), stripeIx);
        return false;
    }
    if (stripeIx < m_stripesCount - 1)
        m_doorPosition = static_cast<DoorPosition>((static_cast<double>(stripeIx) * m_doorPositionMax) / m_stripesCount);
    else
        m_doorPosition = m_doorPositionMax;
    return true;
}

bool Controller::setDoorStripesCount(int stripesCount)
{
    if (stripesCount < 2 || stripesCount > c_maxDoorStripes) {
        APP_LOG_ERROR(_T("Invalid stripes count %d - must be from <2; %u>"), stripesCount, c_maxDoorStripes);
        return false;
    }
    if (isConnected()) {
        APP_LOG_ERROR(_T("Unable to set door stripes count to %d while connected"), stripesCount);
        return false;
    }
    m_stripesCount = stripesCount;
    m_doorPositionMax = stripesCount * c_stepsPerStripe;
    return true;
}


bool Controller::setSecondsToClose(int value)
{
    if (value <= 0) {
        APP_LOG_ERROR(_T("Invalid seconds to close %d"), value);
        return false;
    }
    if (isConnected()) {
        APP_LOG_ERROR(_T("Unable to set seconds to close to %d while connected"), value);
        return false;
    }
    m_secondsToClose = value;
    return true;
}

bool Controller::setSecondsToOpen(int value)
{
    if (value <= 0) {
        APP_LOG_ERROR(_T("Invalid seconds to open %d"), value);
        return false;
    }
    if (isConnected()) {
        APP_LOG_ERROR(_T("Unable to set seconds to open to %d while connected"), value);
        return false;
    }
    m_secondsToOpen = value;
    return true;
}


// Implementation


int Controller::_doorStripePosition(DoorPosition from) const noexcept
{
    ATLASSERT(from >= 0);
    ATLASSERT(from <= m_doorPositionMax);
    if (from < m_doorPositionMax)
        return static_cast<int>(static_cast<double>(from) / c_stepsPerStripe);
    return m_stripesCount;
}


void Controller::_markMotorStart() noexcept
{
    m_motorStartTime = std::chrono::high_resolution_clock::now();
    m_doorStartPosition = m_doorPosition;
}


double Controller::_secondsSinceMotorStart() const noexcept
{
    const auto diff = std::chrono::high_resolution_clock::now() - m_motorStartTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() / 1000.;
}

double Controller::_secondsToFullDoorClose() const noexcept
{
    ATLASSERT(m_doorStartPosition <= m_doorPositionMax);
    return (static_cast<double>(m_doorStartPosition) / m_doorPositionMax) * m_secondsToClose;
}

double Controller::_secondsToFullDoorOpen() const noexcept
{
    ATLASSERT(m_doorStartPosition <= m_doorPositionMax);
    return (static_cast<double>(m_doorPositionMax - m_doorStartPosition) / m_doorPositionMax) * m_secondsToOpen;
}

double Controller::_secondsToStepDoorClose(int& nextStripe) const noexcept
{
    ATLASSERT(m_motorState == SteppingDown);
    ATLASSERT(m_doorStartPosition <= m_doorPositionMax);
    nextStripe = _doorStripePosition(m_doorStartPosition);
    ATLASSERT(nextStripe <= getDoorStripesCount());
    if (nextStripe > 0)
        --nextStripe;
    const DoorPosition nextStripePosition = c_stepsPerStripe * nextStripe;
    ATLASSERT(m_doorStartPosition >= nextStripePosition);
    return (static_cast<double>(m_doorStartPosition - nextStripePosition) * m_secondsToClose) /
        (static_cast<double>(c_stepsPerStripe) * m_stripesCount);
}

double Controller::_secondsToStepDoorOpen(int& nextStripe) const noexcept
{
    ATLASSERT(m_motorState == SteppingUp);
    ATLASSERT(m_doorStartPosition <= m_doorPositionMax);
    nextStripe = _doorStripePosition(m_doorStartPosition);
    ATLASSERT(nextStripe <= getDoorStripesCount());
    if (nextStripe < getDoorStripesCount())
        ++nextStripe;
    const DoorPosition nextStripePosition = c_stepsPerStripe * nextStripe;
    ATLASSERT(m_doorStartPosition <= nextStripePosition);
    return (static_cast<double>(nextStripePosition - m_doorStartPosition) * m_secondsToOpen) /
        (static_cast<double>(c_stepsPerStripe) * m_stripesCount);
}


} // namespace Application
