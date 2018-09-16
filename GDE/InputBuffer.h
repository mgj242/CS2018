#pragma once


#include "common.h"


namespace Application {


class InputBuffer
{
public:
    // Construction & destruction

    InputBuffer();
    ~InputBuffer();


    // Interface

    bool appendBytes(const char*, size_t bytesLength);
    void retrieveLine(String&&);

private:
    // Implementation

    bool _createCurrentLine();


    // Attributes

    std::list<char*> m_lines;
    char* m_currentLine;
    char* m_currentByte;
};


} // namespace Application
