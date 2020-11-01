#include "Runtime/Character/CCharAnimTime.hpp"

#include <algorithm>
#include <cmath>

namespace urde {

bool CCharAnimTime::EqualsZero() const {
  if (x4_type == EType::ZeroIncreasing || x4_type == EType::ZeroSteady || x4_type == EType::ZeroDecreasing)
    return true;

  return x0_time == 0.f;
}

bool CCharAnimTime::EpsilonZero() const { return (std::fabs(x0_time) < 0.00001f); }

bool CCharAnimTime::GreaterThanZero() const {
  if (EqualsZero())
    return false;
  return x0_time > 0.f;
}

bool CCharAnimTime::operator==(const CCharAnimTime& other) const {
  if (x4_type == EType::NonZero) {
    if (other.x4_type == EType::NonZero)
      return x0_time == other.x0_time;
    return false;
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
    return x0_time * other.x0_time > 0.f;

  return false;
}

bool CCharAnimTime::operator!=(const CCharAnimTime& other) const { return !(*this == other); }

bool CCharAnimTime::operator>=(const CCharAnimTime& other) const {
  if (*this == other)
    return true;

  return *this > other;
}

bool CCharAnimTime::operator<=(const CCharAnimTime& other) const {
  if (*this == other)
    return true;

  return *this < other;
}

bool CCharAnimTime::operator>(const CCharAnimTime& other) const { return (!(*this == other) && !(*this < other)); }

bool CCharAnimTime::operator<(const CCharAnimTime& other) const {
  if (x4_type == EType::NonZero) {
    if (other.x4_type == EType::NonZero) {
      return x0_time < other.x0_time;
    }

    return other.EqualsZero() ? x0_time < 0.f : other.x0_time > 0;
  }

  if (!EqualsZero()) {
    if (other.x4_type == EType::Infinity) {
      return x0_time >= 0 || other.x0_time <= 0.f;
    }

    return x0_time < 0.f;
  }

  if (!other.EqualsZero()) {
    if (other.x4_type == EType::NonZero) {
      return other.x0_time > 0.f;
    }

    return other.x0_time > 0.f;
  }

  int type = x4_type == EType::ZeroDecreasing ? -1 : x4_type == EType::ZeroSteady ? 0 : 1;
  int otherType = other.x4_type == EType::ZeroDecreasing ? -1 : other.x4_type == EType::ZeroSteady ? 0 : 1;

  return type < otherType;
}

CCharAnimTime& CCharAnimTime::operator*=(const CCharAnimTime& other) { return *this = *this * other; }

CCharAnimTime& CCharAnimTime::operator+=(const CCharAnimTime& other) { return *this = *this + other; }

CCharAnimTime CCharAnimTime::operator+(const CCharAnimTime& other) const {
  if (x4_type == EType::Infinity && other.x4_type == EType::Infinity) {
    if (other.x0_time != x0_time)
      return {};
    return *this;
  } else if (x4_type == EType::Infinity)
    return *this;
  else if (other.x4_type == EType::Infinity)
    return other;

  if (!EqualsZero() || !other.EqualsZero())
    return {x0_time + other.x0_time};

  int type = -1;
  if (x4_type != EType::ZeroDecreasing) {
    type = x4_type == EType::ZeroSteady ? 0 : 1;
  }

  int otherType = -1;
  if (other.x4_type != EType::ZeroDecreasing) {
    otherType = other.x4_type == EType::ZeroSteady ? 0 : 1;
  }

  type += otherType;
  otherType = std::max(-1, std::min(type, 1));

  if (otherType == -1)
    return {EType::ZeroDecreasing, 0.f};
  else if (otherType == 0)
    return {EType::ZeroSteady, 0.f};
  else
    return {EType::ZeroIncreasing, 0.f};
}

CCharAnimTime& CCharAnimTime::operator-=(const CCharAnimTime& other) { return *this = *this - other; }

CCharAnimTime CCharAnimTime::operator-(const CCharAnimTime& other) const {
  if (x4_type == EType::Infinity && other.x4_type == EType::Infinity) {
    if (other.x0_time == x0_time)
      return {};
    return *this;
  } else if (x4_type == EType::Infinity)
    return *this;
  else if (other.x4_type == EType::Infinity) {
    return {EType::Infinity, -other.x0_time};
  }

  if (!EqualsZero() || !other.EqualsZero())
    return {x0_time - other.x0_time};

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

  type -= otherType;
  if (type == -1)
    return {EType::ZeroDecreasing, 0.f};
  else if (type == 0)
    return {EType::ZeroSteady, 0.f};
  else
    return {EType::ZeroIncreasing, 0.f};
}

CCharAnimTime CCharAnimTime::operator*(const CCharAnimTime& other) const {
  if (x4_type == EType::Infinity && other.x4_type == EType::Infinity) {
    if (other.x0_time != x0_time)
      return {};
    return *this;
  } else if (x4_type == EType::Infinity)
    return *this;
  else if (other.x4_type == EType::Infinity)
    return other;

  if (!EqualsZero() || !other.EqualsZero())
    return {x0_time * other.x0_time};

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

  if (otherType == -1)
    return {EType::ZeroDecreasing, 0.f};
  else if (otherType == 0)
    return {EType::ZeroSteady, 0.f};
  else
    return {EType::ZeroIncreasing, 0.f};
}

CCharAnimTime CCharAnimTime::operator*(const float& other) const {
  if (other == 0.f)
    return {};

  if (!EqualsZero())
    return {x0_time * other};

  if (other > 0.f)
    return *this;

  if (x4_type == EType::ZeroDecreasing) {
    return {EType::ZeroIncreasing, 0.f};
  } else if (x4_type == EType::ZeroSteady) {
    return {EType::ZeroSteady, 0.f};
  } else {
    return {EType::ZeroDecreasing, 0.f};
  }
}

float CCharAnimTime::operator/(const CCharAnimTime& other) const {
  if (other.EqualsZero())
    return 0.f;

  return x0_time / other.x0_time;
}

} // namespace urde
