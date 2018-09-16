#include "stdafx.h"

#include "InputBuffer.h"
#include "Log.h"


namespace Application {


// Construction & destruction


InputBuffer::InputBuffer() :
    m_currentLine(nullptr)
,   m_currentByte(nullptr)
{
}


InputBuffer::~InputBuffer()
{
    for (auto i = m_lines.begin(); i != m_lines.end(); ++i)
        delete[] *i;
    if (m_currentLine)
        delete[] m_currentLine;
}


// Interface


bool InputBuffer::appendBytes(const char* bytes, size_t bytesLength)
{
    ATLENSURE(bytes != nullptr);
    ATLENSURE(bytesLength > 0);
    ATLENSURE(bytesLength <= c_maxLineLength);

    if (m_currentLine == nullptr && !_createCurrentLine())
        return false;
    ATLASSERT(m_currentByte != nullptr);
    ATLASSERT(m_currentLine <= m_currentByte);

    const size_t currentLength = m_currentByte - m_currentLine;
    if (currentLength + bytesLength <= c_maxLineLength) {
        for (size_t i = 0; i < bytesLength; ++i) {
            if (bytes[i] == c_newLineEndDelimiter) {
                *m_currentByte = '\0';
                m_lines.push_back(m_currentLine);
                if (!_createCurrentLine())
                    return false;
            } else
                *(m_currentByte++) = bytes[i];
        }
        return true;
    } else {
        String bytesString = stringFromUtf8(bytes, gsl::narrow<int>(bytesLength));
        APP_LOG_ERROR(_T("Unable to append line bytes \"%s\" of length %lu to first line of input buffer with length %lu"),
            bytesString.c_str(), bytesLength, currentLength);
        return false;
    }
    return true;
}

void InputBuffer::retrieveLine(String&& line)
{
    if (!m_lines.empty()) {
        char* origLine = *m_lines.begin();
        line = stringFromUtf8(origLine, gsl::narrow<int>(strlen(origLine)));
        delete[] origLine;
        m_lines.pop_front();
    }
}


// Implementation


bool InputBuffer::_createCurrentLine()
{
    try {
        m_currentLine = m_currentByte = new char[c_maxLineLength];
    }
    catch (const std::bad_alloc& error) {
        APP_LOG_ERROR(_T("Unable to allocate input buffer line of length %lu: %s"), c_maxLineLength, error.what());
        return false;
    }
    return true;
}


} // namespace Application
