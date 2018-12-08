#include "CCharAnimTime.hpp"
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace urde {

CCharAnimTime CCharAnimTime::Infinity() {
  CCharAnimTime ret(1.f);
  ret.x4_type = EType::Infinity;
  return ret;
}

bool CCharAnimTime::EqualsZero() const {
  if (x4_type == EType::ZeroIncreasing || x4_type == EType::ZeroSteady || x4_type == EType::ZeroDecreasing)
    return true;

  return (x0_time == 0.f);
}

bool CCharAnimTime::EpsilonZero() const { return (std::fabs(x0_time) < 0.00001f); }

bool CCharAnimTime::GreaterThanZero() const {
  if (EqualsZero())
    return false;
  return (x0_time > 0.f);
}

bool CCharAnimTime::operator==(const CCharAnimTime& other) const {
  if (x4_type == EType::NonZero) {
    if (other.x4_type == EType::NonZero)
      return x0_time == other.x0_time;
    return !other.EqualsZero();
  }

  if (EqualsZero()) {
    if (other.EqualsZero()) {
      int type = -1;
      if (x4_type != EType::ZeroDecreasing) {
        if (x4_type != EType::ZeroSteady)
          type = 1;
        else
          type = 0;
      }

      int otherType = -1;
      if (other.x4_type != EType::ZeroDecreasing) {
        if (other.x4_type != EType::ZeroSteady)
          otherType = 1;
        else
          otherType = 0;
      }

      return type == otherType;
    }
    return false;
  }

  if (other.x4_type == EType::Infinity)
    return x0_time * other.x0_time < 0.f;

  return false;
}

bool CCharAnimTime::operator!=(const CCharAnimTime& other) const { return !(*this == other); }

bool CCharAnimTime::operator>=(const CCharAnimTime& other) const {
  if (*this == other)
    return true;

  return (*this > other);
}

bool CCharAnimTime::operator<=(const CCharAnimTime& other) const {
  if (*this == other)
    return true;

  return (*this < other);
}

bool CCharAnimTime::operator>(const CCharAnimTime& other) const { return (!(*this == other) && !(*this < other)); }

bool CCharAnimTime::operator<(const CCharAnimTime& other) const {
  if (x4_type == EType::NonZero) {
    if (other.x4_type == EType::NonZero)
      return x0_time < other.x0_time;
    if (other.EqualsZero())
      return x0_time < 0.f;
    else
      return other.x0_time > 0.f;
  }

  if (EqualsZero()) {
    if (other.EqualsZero()) {
      int type = -1;
      if (x4_type != EType::ZeroDecreasing) {
        if (x4_type != EType::ZeroSteady)
          type = 1;
        else
          type = 0;
      }

      int otherType = -1;
      if (other.x4_type != EType::ZeroDecreasing) {
        if (other.x4_type != EType::ZeroSteady)
          otherType = 1;
        else
          otherType = 0;
      }

      return type < otherType;
    }

    if (other.x4_type == EType::NonZero)
      return other.x0_time > 0.f;
    return other.x0_time < 0.f;
  } else {
    if (x4_type == EType::Infinity)
      return x0_time < 0.f && other.x0_time > 0.f;
    return x0_time < other.x0_time;
  }
}

CCharAnimTime& CCharAnimTime::operator*=(const CCharAnimTime& other) {
  *this = *this * other;
  return *this;
}

CCharAnimTime& CCharAnimTime::operator+=(const CCharAnimTime& other) {
  *this = *this + other;
  return *this;
}

CCharAnimTime CCharAnimTime::operator+(const CCharAnimTime& other) const {
  if (x4_type == EType::Infinity && other.x4_type == EType::Infinity) {
    if (other.x0_time != x0_time)
      return CCharAnimTime();
    return *this;
  } else if (x4_type == EType::Infinity)
    return *this;
  else if (other.x4_type == EType::Infinity)
    return other;

  if (!EqualsZero() || !other.EqualsZero())
    return CCharAnimTime(x0_time + other.x0_time);

  int type = -1;
  if (x4_type != EType::ZeroDecreasing) {
    if (x4_type != EType::ZeroSteady)
      type = 1;
    else
      type = 0;
  }

  int otherType = -1;
  if (other.x4_type != EType::ZeroDecreasing) {
    if (other.x4_type != EType::ZeroSteady)
      otherType = 1;
    else
      otherType = 0;
  }

  type += otherType;
  otherType = std::max(-1, std::min(type, 1));

  CCharAnimTime ret;
  if (otherType == -1)
    ret.x4_type = EType::ZeroDecreasing;
  else if (otherType == 0)
    ret.x4_type = EType::ZeroSteady;
  else
    ret.x4_type = EType::ZeroIncreasing;

  return ret;
}

CCharAnimTime& CCharAnimTime::operator-=(const CCharAnimTime& other) {
  *this = *this - other;
  return *this;
}

CCharAnimTime CCharAnimTime::operator-(const CCharAnimTime& other) const {
  if (x4_type == EType::Infinity && other.x4_type == EType::Infinity) {
    if (other.x0_time == x0_time)
      return CCharAnimTime();
    return *this;
  } else if (x4_type == EType::Infinity)
    return *this;
  else if (other.x4_type == EType::Infinity) {
    CCharAnimTime ret(-other.x0_time);
    ret.x4_type = EType::Infinity;
    return ret;
  }

  if (!EqualsZero() || !other.EqualsZero())
    return CCharAnimTime(x0_time - other.x0_time);

  int type = -1;
  if (x4_type != EType::ZeroDecreasing) {
    if (x4_type != EType::ZeroSteady)
      type = 1;
    else
      type = 0;
  }

  int otherType = -1;
  if (other.x4_type != EType::ZeroDecreasing) {
    if (other.x4_type != EType::ZeroSteady)
      otherType = 1;
    else
      otherType = 0;
  }

  CCharAnimTime ret;
  type -= otherType;
  if (type == -1)
    ret.x4_type = EType::ZeroDecreasing;
  else if (type == 0)
    ret.x4_type = EType::ZeroSteady;
  else
    ret.x4_type = EType::ZeroIncreasing;

  return ret;
}

CCharAnimTime CCharAnimTime::operator*(const CCharAnimTime& other) const {
  if (x4_type == EType::Infinity && other.x4_type == EType::Infinity) {
    if (other.x0_time != x0_time)
      return CCharAnimTime();
    return *this;
  } else if (x4_type == EType::Infinity)
    return *this;
  else if (other.x4_type == EType::Infinity)
    return other;

  if (!EqualsZero() || !other.EqualsZero())
    return CCharAnimTime(x0_time * other.x0_time);

  int type = -1;
  if (x4_type != EType::ZeroDecreasing) {
    if (x4_type != EType::ZeroSteady)
      type = 1;
    else
      type = 0;
  }

  int otherType = -1;
  if (other.x4_type != EType::ZeroDecreasing) {
    if (other.x4_type != EType::ZeroSteady)
      otherType = 1;
    else
      otherType = 0;
  }

  type += otherType;
  otherType = std::max(-1, std::min(type, 1));

  CCharAnimTime ret;
  if (otherType == -1)
    ret.x4_type = EType::ZeroDecreasing;
  else if (otherType == 0)
    ret.x4_type = EType::ZeroSteady;
  else
    ret.x4_type = EType::ZeroIncreasing;
  return ret;
}

CCharAnimTime CCharAnimTime::operator*(const float& other) const {
  CCharAnimTime ret;
  if (other == 0.f)
    return ret;

  if (!EqualsZero())
    return CCharAnimTime(x0_time * other);

  if (other > 0.f)
    return *this;
  else if (other == 0.f)
    return ret;

  ret.x4_type = x4_type;
  return ret;
}

float CCharAnimTime::operator/(const CCharAnimTime& other) const {
  if (other.EqualsZero())
    return 0.f;

  return x0_time / other.x0_time;
}

} // namespace urde
