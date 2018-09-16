#include "stdafx.h"

#include "Log.h"
#include "SerialPort.h"


namespace Application {


// Construction & destruction


SerialPort::SerialPort() :
    m_handle(nullptr)
{
}

SerialPort::~SerialPort()
{
}


// Interface


const SerialPort::PortsAndNamesArray& SerialPort::enumerateComPorts()
{
    // obtain COM ports IDs with their descriptive names, if possible
    m_portsAndNamesArray.clear();

    size_t foundCount = 0;

    // com2com could be used to create debugging COM port pairs, use compatible detection methods first
    CEnumerateSerial::CPortsArray ports;
    CEnumerateSerial::CPortAndNamesArray portsAndNames;
    HRESULT enumResult = CEnumerateSerial::UsingWMI(portsAndNames);
    if (SUCCEEDED(enumResult)) {
        foundCount = portsAndNames.size();
        APP_LOG_DEBUG(_T("Found %u COM ports using WMI"), foundCount);
        if (foundCount > 0) {
            m_portsAndNamesArray = portsAndNames;
        } else {
            APP_LOG_WARNING(_T("No COM ports found using WMI enumeration"));
        }
    } else {
        APP_LOG_WARNING(_T("WMI COM ports enumeration failed with 0x%08lX"), enumResult);
    }

    if (foundCount <= 0) {
        if (CEnumerateSerial::UsingQueryDosDevice(ports)) {
            foundCount = ports.size();
            APP_LOG_DEBUG(_T("Found %u COM ports using QueryDosDevice"), foundCount);
            if (foundCount > 0) {
                for (auto i = ports.begin(); i != ports.end(); ++i) {
                    m_portsAndNamesArray.push_back(std::make_pair(*i, _portNameOf(*i)));
                }
            } else {
                APP_LOG_WARNING(_T("No COM ports found using QueryDosDevice enumeration"));
            }
        } else {
            APP_LOG_WARNING(_T("QueryDosDevice COM ports enumeration failed"));
        }
    }
        
    if (foundCount <= 0) {
        if (!CEnumerateSerial::UsingCreateFile(ports)) {
            foundCount = ports.size();
            APP_LOG_DEBUG(_T("Found %u COM ports using CreateFile"), foundCount);
            if (foundCount > 0) {
                for (auto i = ports.begin(); i != ports.end(); ++i) {
                    m_portsAndNamesArray.push_back(std::make_pair(*i, _portNameOf(*i)));
                }
            } else {
                APP_LOG_WARNING(_T("No COM ports found using CreateFile enumeration"));
            }
        } else {
            APP_LOG_WARNING(_T("CreateFile COM ports enumeration failed"));
        }
    }

    if (foundCount <= 0) {
        if (!CEnumerateSerial::UsingGetDefaultCommConfig(ports)) {
            foundCount = ports.size();
            APP_LOG_DEBUG(_T("Found %u COM ports using CreateFile"), foundCount);
            if (foundCount > 0) {
                for (auto i = ports.begin(); i != ports.end(); ++i) {
                    m_portsAndNamesArray.push_back(std::make_pair(*i, _portNameOf(*i)));
                }
            } else {
                APP_LOG_WARNING(_T("No COM ports found using CreateFile enumeration"));
            }
        } else {
            APP_LOG_WARNING(_T("GetDefaultCommConfig COM ports enumeration failed"));
        }
    }

    // following calls might not find com2com ports

    if (foundCount <= 0) {
        if (!CEnumerateSerial::UsingEnumPorts(portsAndNames)) {
            foundCount = portsAndNames.size();
            APP_LOG_DEBUG(_T("Found %u COM ports using EnumPorts"), foundCount);
            if (foundCount > 0) {
                m_portsAndNamesArray = portsAndNames;
            } else {
                APP_LOG_WARNING(_T("No COM ports found using EnumPorts enumeration"));
            }
        } else {
            APP_LOG_WARNING(_T("EnumPorts COM ports enumeration failed"));
        }
    }

    if (foundCount <= 0) {
        if (!CEnumerateSerial::UsingComDB(ports)) {
            foundCount = ports.size();
            APP_LOG_DEBUG(_T("Found %u COM ports using ComDB"), foundCount);
            if (foundCount > 0) {
                for (auto i = ports.begin(); i != ports.end(); ++i) {
                    m_portsAndNamesArray.push_back(std::make_pair(*i, _portNameOf(*i)));
                }
            } else {
                APP_LOG_WARNING(_T("No COM ports found using ComDB enumeration"));
            }
        } else {
            APP_LOG_WARNING(_T("ComDB COM ports enumeration failed"));
        }
    }

    if (foundCount <= 0) {
        if (!CEnumerateSerial::UsingGetCommPorts(ports)) {
            foundCount = ports.size();
            APP_LOG_DEBUG(_T("Found %u COM ports using GetCommPorts"), foundCount);
            if (foundCount > 0) {
                for (auto i = ports.begin(); i != ports.end(); ++i) {
                    m_portsAndNamesArray.push_back(std::make_pair(*i, _portNameOf(*i)));
                }
            } else {
                APP_LOG_WARNING(_T("No COM ports found using GetCommPorts enumeration"));
            }
        } else {
            APP_LOG_WARNING(_T("GetCommPorts COM ports enumeration failed"));
        }
    }

    std::sort(m_portsAndNamesArray.begin(), m_portsAndNamesArray.end(), [](const auto& left, const auto&right) {
        return left.first < right.first;
    });
    return m_portsAndNamesArray;
}


void SerialPort::close()
{
    if (m_handle != nullptr) {
        if (!CloseHandle(m_handle)) {
            APP_LOG_ERROR(_T("Closing of COM port handle failed with error %lu"), GetLastError());
        }
        m_handle = nullptr;
        APP_LOG_INFO(_T("Serial port closed"));
    }
}

bool SerialPort::open(UINT port)
{
    ATLASSERT(m_handle == nullptr);

    // construct COM port path
    OStringStream buffer;
    buffer << "\\\\.\\" << c_portNamePrefix << port;
    String portPath = buffer.str();
    APP_LOG_INFO(_T("Opening port \"%s\""), portPath.c_str());

    // open the port for reading and writing
    m_handle = CreateFile(portPath.c_str(), GENERIC_READ | GENERIC_WRITE, /*sharing*/0/*none*/,
        /*security*/nullptr/*default*/, OPEN_EXISTING, /*flags*/FILE_ATTRIBUTE_NORMAL, /*template*/nullptr);
    if (m_handle == INVALID_HANDLE_VALUE) {
        APP_LOG_ERROR(_T("Opening of COM port \"%s\" failed with error %lu"), portPath.c_str(), GetLastError());
        close();
        return false;
    }
    ATLASSERT(m_handle != nullptr);

    // configure COM port
    DCB portConfig;
    portConfig.BaudRate = CBR_9600;
    portConfig.ByteSize = 8;
    portConfig.DCBlength = sizeof(DCB);
    portConfig.EofChar = '\0';
    portConfig.ErrorChar = '?';
    portConfig.EvtChar = '\0';
    portConfig.fAbortOnError = TRUE;
    portConfig.fBinary = TRUE;
    portConfig.fDsrSensitivity = FALSE; // no DSR
    portConfig.fDtrControl = DTR_CONTROL_DISABLE;
    portConfig.fErrorChar = TRUE; // replace errors with ErrorChar, although also using fAbortOnError
    portConfig.fInX = FALSE; // Xon/Xoff not used
    portConfig.fNull = FALSE; // do not discard NUL bytes
    portConfig.fOutX = FALSE; // Xon/Xoff not used
    portConfig.fOutxCtsFlow = FALSE; // no CTS
    portConfig.fOutxDsrFlow = FALSE; // no DTX
    portConfig.fParity = TRUE;
    portConfig.fRtsControl = RTS_CONTROL_DISABLE;
    portConfig.fTXContinueOnXoff = FALSE; // Xon/Xoff not used
    portConfig.Parity = ODDPARITY;
    portConfig.StopBits = ONESTOPBIT;
    portConfig.XoffChar = '\0'; // Xon/Xoff not used
    portConfig.XoffLim = 0; // Xon/Xoff not used
    portConfig.XonChar = '\0'; // Xon/Xoff not used
    portConfig.XonLim = 0; // Xon/Xoff not used

    if (!SetCommState(m_handle, &portConfig)) {
        APP_LOG_ERROR(_T("Configuring of COM port \"%s\" failed with error %lu"), portPath.c_str(), GetLastError());
        close();
        return false;
    }
    const TCHAR* parity = nullptr;
    switch (portConfig.Parity) {
    case NOPARITY:      parity = _T("none"); break;
    case ODDPARITY:     parity = _T("odd"); break;
    case EVENPARITY:    parity = _T("even"); break;
    case MARKPARITY:    parity = _T("mark"); break;
    case SPACEPARITY:   parity = _T("space"); break;
    default:
        ATLASSERT(false/*unknown*/);
    }
    APP_LOG_DEBUG(_T("COM port settings: baud rate %d, %s parity, byte size %d, %s stop bits"), portConfig.BaudRate,
        parity, portConfig.ByteSize, portConfig.StopBits == ONESTOPBIT ? _T("1") : portConfig.StopBits == ONE5STOPBITS ? _T("1.5") : _T("2"));

    // set COM port timeouts
    COMMTIMEOUTS portTimeouts;
    portTimeouts.ReadIntervalTimeout = MAXDWORD; // non-blocking
    portTimeouts.ReadTotalTimeoutConstant = 0; // unused
    portTimeouts.ReadTotalTimeoutMultiplier = 0; // unused
    portTimeouts.WriteTotalTimeoutConstant = 50/*ms*/;
    portTimeouts.WriteTotalTimeoutMultiplier = 10/*ms*/;

    if (!SetCommTimeouts(m_handle, &portTimeouts)) {
        APP_LOG_ERROR(_T("Setting of COM port \"%s\" timeouts failed with error %lu"), portPath.c_str(), GetLastError());
        close();
        return false;
    }

    return true;
}


bool SerialPort::pump()
{
    size_t lineLength;
    const char *line = m_outputBuffer.peekFirstLine(lineLength);
    if (line == nullptr)
        return true;

    // write the buffer with line to COM port, update output buffer position with respect to bytes written
    DWORD bytesWritten;
    if (!WriteFile(m_handle, line, static_cast<DWORD>(lineLength), &bytesWritten, /*overlapped*/nullptr)) {
        APP_LOG_ERROR(_T("Unable to write line \"%s\" to COM port, failed with: 0x%08X"), line, GetLastError());
        return false;
    }
    if (bytesWritten > 0)
        return m_outputBuffer.removeBytesFromFirstLine(bytesWritten);

    return true;
}

bool SerialPort::readLine(String&& line)
{
    char buffer[c_maxLineLength];
    DWORD bytesRed = 0;
    if (!ReadFile(m_handle, buffer, c_maxLineLength, &bytesRed, /*overlapped*/nullptr)) {
        APP_LOG_ERROR(_T("Unable to read from COM port, failed with: 0x%08X"), GetLastError());
        return false;
    }

    if (bytesRed > 0) {
        if (!m_inputBuffer.appendBytes(buffer, bytesRed)) {
            String bytesString = stringFromUtf8(buffer, bytesRed);
            APP_LOG_ERROR(_T("Unable to append line \"%s\" to COM port input buffer"), bytesString.c_str());
            return false;
        }
        m_inputBuffer.retrieveLine(std::move(line));
    }

    return true;
}

bool SerialPort::writeLine(const String& newLine)
{
    // check the line for the presence of EOL characters
    if (newLine.find_first_of(c_newLineDelimiters) != String::npos) {
        APP_LOG_ERROR(_T("Line \"%s\" contains one or more of newline delimiters \"%s\" - unable to write it to the COM port"),
            newLine.c_str(), c_newLineDelimiters);
        return false;
    }

    // put the line into buffer, then extract first line to be written to COM port
    if (!m_outputBuffer.appendLine(newLine, c_newLineEndDelimiter)) {
        APP_LOG_ERROR(_T("Unable to append line \"%s\" to COM port output buffer"), newLine.c_str());
        return false;
    }

    return pump();
}


// Private constants


const TCHAR* const SerialPort::c_portNamePrefix = _T("COM");

const TCHAR SerialPort::c_newLineDelimiters[] = { _T('\r'), c_newLineEndDelimiter, '\0' };


// Implementation


String SerialPort::_portNameOf(UINT port)
{
    OStringStream buf;
    buf << c_portNamePrefix << port;
    return buf.str();
}


} // namespace Application
