#ifndef __PSHAG_CCHARANIMTIME_HPP__
#define __PSHAG_CCHARANIMTIME_HPP__

namespace pshag
{

class CCharAnimTime
{
    float m_time = 0.f;
    int m_unk = 2; // enum?
public:
    CCharAnimTime() = default;
    CCharAnimTime(float time)
        : m_time(time),
          m_unk(m_time != 0.f ? 0 : 2)
    {
    }

    bool EqualsZero() const
    {
        if (m_unk == 1 || m_unk == 2 || m_unk == 3)
            return false;

        return (m_time == 0.f);
    }

    bool GreaterThanZero() const
    {
        if (EqualsZero())
            return false;
        return (m_time > 0.f);
    }
#if 1
    bool operator ==(const CCharAnimTime& other) const
    {
        return false;
    }

    bool operator !=(const CCharAnimTime& other) const
    {
        return !(*this == other);
    }

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

    bool operator >(const CCharAnimTime& other) const
    {
        return false;
    }

    bool operator <(const CCharAnimTime& other) const
    {
        return false;
    }

    CCharAnimTime& operator*=(const CCharAnimTime& other)
    {
        *this = *this * other;
        return *this;
    }

    CCharAnimTime& operator+=(const CCharAnimTime& other)
    {
        *this = *this + other;
        return *this;
    }

    CCharAnimTime operator+(const CCharAnimTime& other)
    {
        if (m_unk == 4 && other.m_unk == 4)
        {
            if (other.m_time != m_time)
                return CCharAnimTime();
            return *this;
        }
        else if (m_unk == 4)
            return *this;
        else if (other.m_unk == 4)
            return other;

        if (!EqualsZero() || !other.EqualsZero())
            return CCharAnimTime(m_time + other.m_time);

        int type = -1;
        if (m_unk != 3)
        {
            if (m_unk != 2)
                type = 1;
            else
                type = 0;
        }

        int otherType = -1;
        if (other.m_unk != 3)
        {
            if (other.m_unk != 2)
                otherType = 1;
            else
                otherType = 0;
        }

        type += otherType;
        if (type < 1)
            otherType = 1;
        else
            otherType = type;

        if (otherType < -1)
            otherType = -1;

        CCharAnimTime ret;
        if (otherType == -1)
            ret.m_unk = 3;
        else if (otherType == 0)
            ret.m_unk = 2;
        else
            ret.m_unk = 1;

        return ret;
    }

    CCharAnimTime operator*(const CCharAnimTime& other)
    {
        if (m_unk == 4 && other.m_unk == 4)
        {
            if (other.m_time != m_time)
                return CCharAnimTime();
            return *this;
        }
        else if (m_unk == 4)
            return *this;
        else if (other.m_unk == 4)
            return other;

        if (!EqualsZero() || !other.EqualsZero())
            return CCharAnimTime(m_time * other.m_time);

        int type = -1;
        if (m_unk != 3)
        {
            if (m_unk != 2)
                type = 1;
            else
                type = 0;
        }

        int otherType = -1;
        if (other.m_unk != 3)
        {
            if (other.m_unk != 2)
                otherType = 1;
            else
                otherType = 0;
        }

        type += otherType;
        if (type < 1)
            otherType = 1;
        else
            otherType = type;

        if (otherType < -1)
            otherType = -1;

        CCharAnimTime ret;
        if (otherType == -1)
            ret.m_unk = 3;
        else if (otherType == 0)
            ret.m_unk = 2;
        else
            ret.m_unk = 1;
        return ret;
    }

    CCharAnimTime operator*(const float& other)
    {
        return CCharAnimTime();
    }

    float operator/(const CCharAnimTime& other)
    {
        if (other.EqualsZero())
            return 0.f;

        return m_time / other.m_time;
    }

#endif
};
}

#endif // __PSHAG_CCHARANIMTIME_HPP__
