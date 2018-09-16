#pragma once


class ReentrancyGuard
{
public:
    // Construction & destruction

    ReentrancyGuard(bool& flag) :
        m_flag(flag)
    {
        ATLASSERT(m_flag);
        m_flag = false;
    }

    ~ReentrancyGuard() {
        ATLASSERT(!m_flag);
        m_flag = true;
    }

private:
    // Attributes

    bool& m_flag;
};


#define REENTRANCY_GUARD() \
    static bool _reetrancyGuardFlag = true; \
    if (!_reetrancyGuardFlag) \
        return; \
    ReentrancyGuard _reentrancyGuard(_reetrancyGuardFlag);
