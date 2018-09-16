#pragma once


#include "enumser.h"

#include "common.h"
#include "InputBuffer.h"
#include "OutputBuffer.h"


namespace Application {


class SerialPort
{
public:
    // Types

    typedef CEnumerateSerial::CPortAndNamesArray PortsAndNamesArray;


    // Construction & destruction

    SerialPort();
    ~SerialPort();


    // Interface

    bool open(UINT port);
    void close();

    bool pump();
    bool readLine(String&&);
    bool writeLine(const String&);

    const PortsAndNamesArray& enumerateComPorts();

private:
    // Constants

    static const TCHAR* const c_portNamePrefix;

    static const TCHAR c_newLineDelimiters[];


    // Implementation

    static String _portNameOf(UINT port);


    // Attributes

    PortsAndNamesArray m_portsAndNamesArray;

    HANDLE m_handle;

    InputBuffer m_inputBuffer;
    OutputBuffer m_outputBuffer;
};


} // namespace Application
