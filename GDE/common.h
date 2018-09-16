#pragma once


// Types


#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif


namespace Application {


typedef std::basic_istringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> IStringStream;
typedef std::basic_ostringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> OStringStream;
typedef std::basic_stringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> StringStream;


typedef std::chrono::time_point< std::chrono::high_resolution_clock> TimePoint;


enum Command
{
    Nop,
    FullClose,
    CloseToNextStripe,
    FullOpen,
    OpenToNextStripe,
    StopMotor,
    IndoorLightsOn,
    IndoorLightsOff,
    OutdoorLightsOn,
    OutoorLightsOff
};

enum MotorState
{
    Stopped,
    Blocked,
    RunningDown,
    RunningUp,
    SteppingDown,
    SteppingUp
};


// Constants


enum { c_maxLineLength = 1024 };
enum { c_newLineEndDelimiter = _T('\n') };


// Functions


int utf8StringLength(const String& string);

bool stringToUtf8(const String& input, int resultLength, char* result);
bool stringToUtf8(const String& input, std::string& result);
String stringFromUtf8(const char* input, int length);


} // namespace Application
