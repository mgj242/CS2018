#pragma once


#include "ColorListBox.h"

#include "common.h"


namespace Application {


#define _APP_LOG_MESSAGE(severity, message, ...) \
    Application::Log::getInstance().addMessage(Application::Log::severity, message, __VA_ARGS__)

#define APP_LOG_ERROR(message, ...) \
    _APP_LOG_MESSAGE(Error, message, __VA_ARGS__)
#define APP_LOG_WARNING(message, ...) \
    _APP_LOG_MESSAGE(Warning, message, __VA_ARGS__)
#define APP_LOG_INFO(message, ...) \
    _APP_LOG_MESSAGE(Info, message, __VA_ARGS__)
#define APP_LOG_DEBUG(message, ...) \
    _APP_LOG_MESSAGE(Debug, message, __VA_ARGS__)


class Log
{
public:
    // Types

    enum Severity {
        Debug, // lowest severity first
        Info,
        Warning,
        Error
    };


    // Construction & destruction

    Log();
    ~Log();

    void initialize(CColorListBox* logListBox, CComboBox* filterComboBox, CButton* followCheckBox);

    static Log& getInstance();


    // Interface

    void addMessage(Severity, const TCHAR*, ...);
    void clear();


    // Accessors

    void setFilter(Severity);
    void setFollow(bool);

private:
    // Types

    struct Message {
        Severity severity;
        String text;
    };


    // Constants

    static const size_t c_messageBufferSize = 1024;


    // Implementation

    void _clear();

    COLORREF _colorForSeverity(Severity);


    // Attributes

    CColorListBox* m_listBox;

    Severity m_filter;
    bool m_follow;

    std::vector<Message> m_messages;
};


} // namespace Application
