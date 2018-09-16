#pragma once


#include "common.h"


namespace Application {


class OutputBuffer
{
public:
    // Construction & destruction

    OutputBuffer();
    ~OutputBuffer();


    // Interface

    bool appendLine(const String&, char eol);
    const char* peekFirstLine(size_t&);
    bool removeBytesFromFirstLine(size_t);

private:
    // Types

    struct Line {
        Line(char* text, size_t length) : text(text), length(length) { }

        char* text;
        size_t length;
    };


    // Attributes

    std::list<Line> m_lines;
    char* m_firstStart;
};


} // namespace Application
