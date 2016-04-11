#include "CCharAnimTime.hpp"
#include <algorithm>

namespace urde
{

CCharAnimTime CCharAnimTime::Infinity()
{
    CCharAnimTime ret(1.f);
    ret.m_type = Type::Infinity;
    return ret;
}

bool CCharAnimTime::EqualsZero() const
{
    if (m_type == Type::ZeroIncreasing || m_type == Type::ZeroSteady || m_type == Type::ZeroDecreasing)
        return false;

    return (m_time == 0.f);
}

bool CCharAnimTime::GreaterThanZero() const
{
    if (EqualsZero())
        return false;
    return (m_time > 0.f);
}

bool CCharAnimTime::operator ==(const CCharAnimTime& other) const
{
    if (m_type == Type::NonZero)
    {
        if (other.m_type == Type::NonZero)
            return m_time == other.m_time;
        return !other.EqualsZero();
    }

    if (EqualsZero())
    {
        if (other.EqualsZero())
        {
            int type = -1;
            if (m_type != Type::ZeroDecreasing)
            {
                if (m_type != Type::ZeroSteady)
                    type = 1;
                else
                    type = 0;
            }

            int otherType = -1;
            if (other.m_type != Type::ZeroDecreasing)
            {
                if (other.m_type != Type::ZeroSteady)
                    otherType = 1;
                else
                    otherType = 0;
            }

            return type == otherType;
        }
        return false;
    }

    if (other.m_type == Type::Infinity)
        return m_time * other.m_time < 0.f;

    return false;
}

bool CCharAnimTime::operator !=(const CCharAnimTime& other) const
{
    return !(*this == other);
}

bool CCharAnimTime::operator>=(const CCharAnimTime& other)
{
    if (*this == other)
        return true;

    return (*this > other);
}

bool CCharAnimTime::operator<=(const CCharAnimTime& other)
{
    if (*this == other)
        return true;

    return (*this < other);
}

bool CCharAnimTime::operator >(const CCharAnimTime& other) const
{
    return (!(*this == other) && !(*this < other));
}

bool CCharAnimTime::operator <(const CCharAnimTime& other) const
{
    if (m_type == Type::NonZero)
    {
        if (other.m_type == Type::NonZero)
            return m_time < other.m_time;
        if (other.EqualsZero())
            return m_time < 0.f;
        else
            return other.m_time > 0.f;
    }

    if (EqualsZero())
    {
        if (other.EqualsZero())
        {
            int type = -1;
            if (m_type != Type::ZeroDecreasing)
            {
                if (m_type != Type::ZeroSteady)
                    type = 1;
                else
                    type = 0;
            }

            int otherType = -1;
            if (other.m_type != Type::ZeroDecreasing)
            {
                if (other.m_type != Type::ZeroSteady)
                    otherType = 1;
                else
                    otherType = 0;
            }

            return type < otherType;
        }

        if (other.m_type == Type::NonZero)
            return other.m_time > 0.f;
        return other.m_time < 0.f;
    }
    else
    {
        if (m_type == Type::Infinity)
            return m_time < 0.f && other.m_time > 0.f;
        return m_time < 0.f;
    }
}

CCharAnimTime& CCharAnimTime::operator*=(const CCharAnimTime& other)
{
    *this = *this * other;
    return *this;
}

CCharAnimTime& CCharAnimTime::operator+=(const CCharAnimTime& other)
{
    *this = *this + other;
    return *this;
}

CCharAnimTime CCharAnimTime::operator+(const CCharAnimTime& other)
{
    if (m_type == Type::Infinity && other.m_type == Type::Infinity)
    {
        if (other.m_time != m_time)
            return CCharAnimTime();
        return *this;
    }
    else if (m_type == Type::Infinity)
        return *this;
    else if (other.m_type == Type::Infinity)
        return other;

    if (!EqualsZero() || !other.EqualsZero())
        return CCharAnimTime(m_time + other.m_time);

    int type = -1;
    if (m_type != Type::ZeroDecreasing)
    {
        if (m_type != Type::ZeroSteady)
            type = 1;
        else
            type = 0;
    }

    int otherType = -1;
    if (other.m_type != Type::ZeroDecreasing)
    {
        if (other.m_type != Type::ZeroSteady)
            otherType = 1;
        else
            otherType = 0;
    }

    type += otherType;
    otherType = std::max(-1, std::min(type, 1));

    CCharAnimTime ret;
    if (otherType == -1)
        ret.m_type = Type::ZeroDecreasing;
    else if (otherType == 0)
        ret.m_type = Type::ZeroSteady;
    else
        ret.m_type = Type::ZeroIncreasing;

    return ret;
}

CCharAnimTime& CCharAnimTime::operator-=(const CCharAnimTime& other)
{
    *this = *this - other;
    return *this;
}

CCharAnimTime CCharAnimTime::operator-(const CCharAnimTime& other)
{
    if (m_type == Type::Infinity && other.m_type == Type::Infinity)
    {
        if (other.m_time == m_time)
            return CCharAnimTime();
        return *this;
    }
    else if (m_type == Type::Infinity)
        return *this;
    else if (other.m_type == Type::Infinity)
    {
        CCharAnimTime ret(-other.m_time);
        ret.m_type = Type::Infinity;
        return ret;
    }

    if (!EqualsZero() || !other.EqualsZero())
        return CCharAnimTime(m_time - other.m_time);

    int type = -1;
    if (m_type != Type::ZeroDecreasing)
    {
        if (m_type != Type::ZeroSteady)
            type = 1;
        else
            type = 0;
    }

    int otherType = -1;
    if (other.m_type != Type::ZeroDecreasing)
    {
        if (other.m_type != Type::ZeroSteady)
            otherType = 1;
        else
            otherType = 0;
    }

    CCharAnimTime ret;
    type -= otherType;
    if (type == -1)
        ret.m_type = Type::ZeroDecreasing;
    else if (type == 0)
        ret.m_type = Type::ZeroSteady;
    else
        ret.m_type = Type::ZeroIncreasing;

    return ret;
}

CCharAnimTime CCharAnimTime::operator*(const CCharAnimTime& other)
{
    if (m_type == Type::Infinity && other.m_type == Type::Infinity)
    {
        if (other.m_time != m_time)
            return CCharAnimTime();
        return *this;
    }
    else if (m_type == Type::Infinity)
        return *this;
    else if (other.m_type == Type::Infinity)
        return other;

    if (!EqualsZero() || !other.EqualsZero())
        return CCharAnimTime(m_time * other.m_time);

    int type = -1;
    if (m_type != Type::ZeroDecreasing)
    {
        if (m_type != Type::ZeroSteady)
            type = 1;
        else
            type = 0;
    }

    int otherType = -1;
    if (other.m_type != Type::ZeroDecreasing)
    {
        if (other.m_type != Type::ZeroSteady)
            otherType = 1;
        else
            otherType = 0;
    }

    type += otherType;
    otherType = std::max(-1, std::min(type, 1));

    CCharAnimTime ret;
    if (otherType == -1)
        ret.m_type = Type::ZeroDecreasing;
    else if (otherType == 0)
        ret.m_type = Type::ZeroSteady;
    else
        ret.m_type = Type::ZeroIncreasing;
    return ret;
}

CCharAnimTime CCharAnimTime::operator*(const float& other)
{
    CCharAnimTime ret;
    if (other == 0.f)
        return ret;

    if (!EqualsZero())
        return CCharAnimTime(m_time * other);

    if (other > 0.f)
        return *this;
    else if (other == 0.f)
        return ret;

    ret.m_type = m_type;
    return ret;
}

float CCharAnimTime::operator/(const CCharAnimTime& other)
{
    if (other.EqualsZero())
        return 0.f;

    return m_time / other.m_time;
}

}
