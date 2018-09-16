#include "stdafx.h"
#include "common.h"

#include "Log.h"


namespace Application {


// Construction & destruction


Log::Log() :
    m_listBox(0)
,   m_filter(Debug)
,   m_follow(true)
{
}


Log::~Log()
{
}


void Log::initialize(CColorListBox* logListBox, CComboBox* filterComboBox, CButton* followCheckBox)
{
    ATLENSURE(logListBox != nullptr);
    ATLENSURE(filterComboBox != nullptr);
    ATLENSURE(followCheckBox != nullptr);
    ATLASSERT(m_listBox == nullptr);

    // repopulate filter combo box, select the lowest severity 
    while (filterComboBox->GetCount() > 0)
        filterComboBox->DeleteString(/*index*/0);
    int lowestItemIx = filterComboBox->AddString(_T("Debug"));
    filterComboBox->SetItemData(lowestItemIx, Debug);
    int lastItemIx = filterComboBox->AddString(_T("Info"));
    filterComboBox->SetItemData(lastItemIx, Info);
    lastItemIx = filterComboBox->AddString(_T("Warning"));
    filterComboBox->SetItemData(lastItemIx, Warning);
    lastItemIx = filterComboBox->AddString(_T("Error"));
    filterComboBox->SetItemData(lastItemIx, Error);
    filterComboBox->SetCurSel(lowestItemIx/*lowest*/);

    // set follow check box as checked
    followCheckBox->SetCheck(BST_CHECKED);

    m_listBox = logListBox;
}


Log& Log::getInstance()
{
    static Log self;
    return self;
}


// Interface



void Log::clear()
{
    m_messages.clear();
    _clear();
}

void Log::addMessage(Severity severity, const TCHAR* text, ...)
{
    ATLENSURE(text);

    // get prefix by severity
    const TCHAR* prefix = 0;
    switch (severity) {
    case Error:     prefix = _T("ERROR"); break;
    case Warning:   prefix = _T("WARNING"); break;
    case Info:      prefix = _T("INFO"); break;
    case Debug:     prefix = _T("DEBUG"); break;
    default:
        ATLASSERT(false/*unexpected severity*/);
    }
    if (prefix == nullptr)
        return;

    // format message
    TCHAR formatBuf[c_messageBufferSize];
    va_list args;
    va_start(args, text);
    _vsntprintf_s(formatBuf, c_messageBufferSize, _TRUNCATE, text, args);
    va_end(args);

    // add timestamp and prefix
    OStringStream msgBuf;
    const auto timeNow = std::chrono::system_clock::now();
    const auto dayStartTime = date::floor<date::days>(timeNow);
    const auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - dayStartTime));
    const auto milliseconds = time.subseconds().count() / 1000.;

    msgBuf << _T('[') << time.hours().count() << _T(':')
        << std::setfill(_T('0')) << std::setw(2) << time.minutes().count() << _T(':')
        << std::fixed << std::setw(2/*digits 00-59*/ + 1/*decimal dot*/ + 2/*precision*/) << std::setprecision(2)
        << time.seconds().count() + milliseconds << _T("] ")
        << prefix << _T(": ") << formatBuf;

    // add formatted message to list box, if its severity passes
    const String message = msgBuf.str();

    m_messages.push_back({severity, message});

    ATLASSERT(m_listBox != nullptr);
    if (severity >= m_filter) {
        int lastMsgIx = m_listBox->AddString(message.c_str(), _colorForSeverity(severity));
        if (m_follow)
            m_listBox->SetCurSel(lastMsgIx > 0 ? lastMsgIx - 1 : 0);
    }

    // add the message to the console
    std::string utf8Message;
    if (stringToUtf8(message, utf8Message)) {
        utf8Message.append("\n");
        OutputDebugStringA(utf8Message.c_str());
    } else
        ATLENSURE(0/*failed*/);
}


// Accessors


void Log::setFilter(Severity filter)
{
    if (m_filter != filter) {
        _clear();

        // add items according to filter
        for (auto i = m_messages.begin(); i != m_messages.end(); ++i)
            if (i->severity >= filter)
                m_listBox->AddString(i->text.c_str(), _colorForSeverity(i->severity));

        m_filter = filter;
    }
}


void Log::setFollow(bool follow)
{
    if (m_follow != follow) {
        if (follow) {
            int msgsCount = m_listBox->GetCount();
            if (msgsCount > 0)
                m_listBox->SetCurSel(msgsCount - 1);
        }
        m_follow = follow;
    }
}


// Implementation


void Log::_clear()
{
    // delete all items in log list box
    ATLASSERT(m_listBox != nullptr);
    while (m_listBox->GetCount() > 0)
        m_listBox->DeleteString(/*index*/0);
}


COLORREF Log::_colorForSeverity(Severity severity)
{
    COLORREF result = RGB(0, 0, 0);
    switch (severity) {
    case Error:     result = RGB(0xFF, 0, 0); break;
    case Warning:   result = RGB(0xFF, 0x66, 0x66); break;
    case Info:      result = RGB(0x66, 0xB2, 0xFF); break;
    case Debug:     result = RGB(0xCC, 0xCC, 0); break;
    default:
        ATLASSERT(false/*unexpected severity*/);
    }
    return result;
}


} // namespace Application
