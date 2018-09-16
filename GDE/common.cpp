#include "stdafx.h"

#include "common.h"
#include "Log.h"


namespace Application {


// Functions


int utf8StringLength(const String& string)
{
    const int result = WideCharToMultiByte(CP_UTF8, /*flags*/0/*replace invalid characters*/, string.c_str(), gsl::narrow<int>(string.size()),
        0/*determine length*/, 0/*determine length*/, /*lpWideCharStr*/0, /*cchWideChar*/0);
    if (result <= 0) {
        APP_LOG_ERROR(_T("Unable to determine length of input string \"%s\" - failed with 0x%08X"), string.c_str(), GetLastError());
    }
    return result;
}


String stringFromUtf8(const char* input, int inLength)
{
    const int outLength = MultiByteToWideChar(CP_ACP, 0, input, inLength, 0/*determine length*/, 0/*determine length*/);
    if (outLength > 0) {
        String result(inLength, /*fill*/_T('\0'));
        MultiByteToWideChar(CP_ACP, 0, input, inLength, &result[0], outLength); // already called above
        return result;
    }
    return String();
}


bool stringToUtf8(const String& input, int resultLength, char* result)
{
    ATLENSURE(resultLength > 0);
    const int inLength = gsl::narrow<int>(input.length());
    if (WideCharToMultiByte(CP_UTF8, 0, input.c_str(), inLength, result, resultLength,
        /*default char*/0, /*default char used*/0) <= 0) {
        APP_LOG_ERROR(_T("Unable to convert input string \"%s\": to UTF8 - failed with 0x%08X"), input.c_str(), GetLastError());
        return false;
    }
    return true;
}

bool stringToUtf8(const String& input, std::string& result)
{
    int resultLength = utf8StringLength(input);
    if (resultLength > 0) {
        result.resize(resultLength);
        return stringToUtf8(input, resultLength, result.data());
    }
    return false;
}


} // namespace Application
