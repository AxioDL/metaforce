#ifndef __PSHAG_CCHARANIMTIME_HPP__
#define __PSHAG_CCHARANIMTIME_HPP__

namespace pshag
{

class CCharAnimTime
{
    float m_time;
    int m_unk; // enum?
public:
    CCharAnimTime(float time)
        : m_time(time),
          m_unk(m_time != 0.0 ? 0 : 2)
    {
    }

    bool EqualsZero()
    {
        if (m_unk == 1 || m_unk == 2 || m_unk == 3)
            return false;

        return (m_time == 0.0);
    }

    bool GreaterThanZero()
    {
        if (EqualsZero())
            return false;
        return (m_time != 0.0);
    }
#if 0
    bool operator>=(const CCharAnimTime& other)
    {
        if (*this == other)
            return true;

        return (*this > other);
    }

    bool operator<=(const CCharAnimTime& other)
    {
        if (*this == other)
            return true;

        return (*this < other);
    }

    void operator*=(const CCharAnimTime& other)
    { *this = *this * other; }

    void operator+=(const CCharAnimTime& other)
    { *this = *this + other; }

    void operator+(const CCharAnimTime& other)
    {
    }
#endif
};
}

#endif // __PSHAG_CCHARANIMTIME_HPP__
