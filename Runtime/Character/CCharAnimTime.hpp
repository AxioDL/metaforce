#pragma once

#include "Runtime/IOStreams.hpp"

#undef min
#undef max

namespace urde {

class CCharAnimTime {
public:
  enum class EType { NonZero, ZeroIncreasing, ZeroSteady, ZeroDecreasing, Infinity };

private:
  float x0_time = 0.f;
  EType x4_type = EType::ZeroSteady;

public:
  CCharAnimTime() = default;
  CCharAnimTime(CInputStream& in) : x0_time(in.readFloatBig()), x4_type(EType(in.readUint32Big())) {}
  CCharAnimTime(float time) : x0_time(time), x4_type(x0_time != 0.f ? EType::NonZero : EType::ZeroSteady) {}

  CCharAnimTime(EType type, const float& t) : x0_time(t), x4_type(type) {}

  static CCharAnimTime Infinity();
  float GetSeconds() const { return x0_time; }

  bool EqualsZero() const;
  bool EpsilonZero() const;
  bool GreaterThanZero() const;
  bool operator==(const CCharAnimTime& other) const;
  bool operator!=(const CCharAnimTime& other) const;
  bool operator>=(const CCharAnimTime& other) const;
  bool operator<=(const CCharAnimTime& other) const;
  bool operator>(const CCharAnimTime& other) const;
  bool operator<(const CCharAnimTime& other) const;
  CCharAnimTime& operator*=(const CCharAnimTime& other);
  CCharAnimTime& operator+=(const CCharAnimTime& other);
  CCharAnimTime operator+(const CCharAnimTime& other) const;
  CCharAnimTime& operator-=(const CCharAnimTime& other);
  CCharAnimTime operator-(const CCharAnimTime& other) const;
  CCharAnimTime operator*(const CCharAnimTime& other) const;
  CCharAnimTime operator*(const float& other) const;
  float operator/(const CCharAnimTime& other) const;
};
} // namespace urde
