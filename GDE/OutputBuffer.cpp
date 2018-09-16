#include "stdafx.h"

#include "Log.h"
#include "OutputBuffer.h"


namespace Application {


// Construction & destruction


OutputBuffer::OutputBuffer() :
    m_firstStart(nullptr)
{
}


OutputBuffer::~OutputBuffer()
{
    for (auto i = m_lines.begin(); i != m_lines.end(); ++i)
        delete[] i->text;
}


// Interface


bool OutputBuffer::appendLine(const String& line, char eol)
{
    // allocate memory for a copy of the line with appended eol character
    int newLength = utf8StringLength(line);
    if (newLength <= 0)
        return false;
    newLength += sizeof(char)/*EOL*/ + 1/*NUL*/;

    char* text = nullptr;
    try { text = new char[newLength]; }
    catch (const std::bad_alloc& error) {
        APP_LOG_ERROR(_T("Unable to allocate line of length %lu: %s"), newLength, error.what());
        return false;
    }

    // convert the line into newly allocated text, append eol
    if (!stringToUtf8(line, newLength, text)) {
        delete[] text;
        return false;
    }
    text[newLength - 2] = eol;
    text[newLength - 1] = '\0';

    // add the line to the strcture
    try {
        if (m_lines.empty())
            m_firstStart = text;
        m_lines.emplace_back(Line(text, newLength));
    }
    catch (const std::bad_alloc& error) {
        APP_LOG_ERROR(_T("Unable to allocate line of length %lu, failed with: %s"), newLength, error.what());
        return false;
    }
    return true;
}


const char* OutputBuffer::peekFirstLine(size_t& remaining)
{
    if (!m_lines.empty()) {
        ATLASSERT(m_firstStart != nullptr);
        remaining = m_lines.begin()->length;
        return m_firstStart;
    }
    ATLASSERT(m_firstStart == nullptr);
    remaining = 0;
    return nullptr;
}


bool OutputBuffer::removeBytesFromFirstLine(size_t count)
{
    if (!m_lines.empty()) {
        ATLASSERT(m_firstStart != nullptr);
        size_t& remaining = m_lines.begin()->length;
        if (remaining < count) {
            APP_LOG_ERROR(_T("Remaining bytes in first line of output buffer %lu < requested count to remove %lu"), remaining, count);
            return false;
        }
        remaining -= count;
        if (remaining == 0) {
            delete[] m_lines.begin()->text;
            m_lines.pop_front();
            if (!m_lines.empty()) {
                m_firstStart = m_lines.begin()->text;
            } else {
                m_firstStart = nullptr;
            }
        }
        return true;
    }
    ATLASSERT(m_firstStart == nullptr);
    return true;
}


} // namespace Application
